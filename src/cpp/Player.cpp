//=============================================================================
//
// プレイヤー処理 [Player.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Player.h"
#include "Texture.h"
#include "Manager.h"
#include "PlayerState.h"
#include "Time.h"
#include "StencilShadow.h"
#include "Tutorial.h"
#include "SpecPlayer.h"
#include "Game.h"

namespace Create
{

}

namespace Respawn
{
	const D3DXVECTOR3 POS{ 0.0f, 30.0f, -300.0f };
}

//=============================================================================
// コンストラクタ
//=============================================================================
CPlayer::CPlayer()
{
	// 値のクリア
	memset(m_apModel, 0, sizeof(m_apModel));			// モデル(パーツ)へのポインタ
	m_mtxWorld			= {};							// ワールドマトリックス
	m_nNumModel			= 0;							// モデル(パーツ)の総数
	m_pShadowS			= nullptr;						// ステンシルシャドウへのポインタ
	m_pMotion			= nullptr;						// モーションへのポインタ
	m_bIsMoving			= false;						// 移動入力フラグ
	m_bOnGround			= false;						// 接地フラグ
	m_pDebug3D			= nullptr;						// 3Dデバッグ表示へのポインタ
	m_canControl		= false;						// 操作フラグ
}
//=============================================================================
// デストラクタ
//=============================================================================
CPlayer::~CPlayer()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CPlayer* CPlayer::Create(D3DXVECTOR3 pos, D3DXVECTOR3 rot)
{
	CPlayer* pPlayer = new CPlayer;

	// nullptrだったら
	if (pPlayer == nullptr)
	{
		return nullptr;
	}

	pPlayer->SetPos(pos);
	pPlayer->SetRot(D3DXToRadian(rot));
	pPlayer->SetSize(D3DXVECTOR3(1.2f, 1.2f, 1.2f));

	// 初期化失敗時
	if (FAILED(pPlayer->Init()))
	{
		return nullptr;
	}

	return pPlayer;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CPlayer::Init(void)
{
	CModel* pModels[MAX_PARTS];
	int nNumModels = 0;

	// パーツの読み込み
	m_pMotion = CMotion::Load("data/MOTION/motion_player.txt", pModels, nNumModels, MAX);

	for (int nCnt = 0; nCnt < nNumModels && nCnt < MAX_PARTS; nCnt++)
	{
		m_apModel[nCnt] = pModels[nCnt];

		// オフセット考慮
		m_apModel[nCnt]->SetOffsetPos(m_apModel[nCnt]->GetPos());
		m_apModel[nCnt]->SetOffsetRot(m_apModel[nCnt]->GetRot());
	}

	// パーツ数を代入
	m_nNumModel = nNumModels;

	// 目標の向きを設定
	SetRotDest(D3DXVECTOR3(0.0f, D3DXToRadian(ROT_DEST), 0.0f));

	// カプセルコライダーの設定
	CreatePhysics(CAPSULE_RADIUS, CAPSULE_HEIGHT, MASS);

	// ステンシルシャドウの生成
	m_pShadowS = CStencilShadow::Create("data/MODELS/stencilshadow.x",VEC3_DEFAULT);
	m_pShadowS->SetStencilRef(1);// 個別のステンシルバッファの参照値を設定

	// インスタンスのポインタを渡す
	m_stateMachine.Start(this);

	// 初期状態のステートをセット
	m_stateMachine.ChangeState<CPlayerStandState>();

	// HPの設定
	SetHp(10.0f);

	//// ゲージを生成
	//SetGuages({ 100.0f, 100.0f, 0.0f }, { 0.0f,1.0f,0.0f,1.0f }, { 1.0f,0.0f,0.0f,1.0f }, 420.0f, 20.0f);

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CPlayer::Uninit(void)
{
	// 当たり判定の破棄
	ReleasePhysics();

	// モデルの破棄
	for (int nCnt = 0; nCnt < MAX_PARTS; nCnt++)
	{
		if (m_apModel[nCnt] != nullptr)
		{
			m_apModel[nCnt]->Uninit();
			delete m_apModel[nCnt];
			m_apModel[nCnt] = nullptr;
		}
	}

	// モーションの破棄
	if (m_pMotion != nullptr)
	{
		delete m_pMotion;
		m_pMotion = nullptr;
	}

	// オブジェクトの破棄(自分自身)
	this->Release();
}
//=============================================================================
// 更新処理
//=============================================================================
void CPlayer::Update(void)
{
	// 名前空間Createの使用
	using namespace Create;

	// カメラの取得
	CCamera* pCamera = CManager::GetCamera();

	// カメラの角度の取得
	D3DXVECTOR3 CamRot = pCamera->GetRot();

	// ステートマシン更新
	m_stateMachine.Update();

	// 入力判定の取得
	InputData input = GatherInput();

//#ifdef _DEBUG
//	// キーボードの取得
//	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();
//
//	if (pKeyboard->GetTrigger(DIK_1))
//	{
//		// ダメージ処理
//		Damage(DEBUG_DAMAGE);
//	}
//	else if (pKeyboard->GetTrigger(DIK_2))
//	{
//		// 回復処理
//		Heal(DEBUG_HEAL);
//	}
//
//#endif
	
	// コライダーの更新処理
	UpdateCollider(D3DXVECTOR3(0.0f, 50.0f, 0.0f));

	// 向きの更新処理
	UpdateRotation(ROT_SPEED);

	// 移動入力があればプレイヤー向きを入力方向に
	if (input.moveDir.x != 0.0f || input.moveDir.z != 0.0f)
	{
		// Y成分だけを使いたいので目標の向きを取得
		D3DXVECTOR3 rotDest = GetRotDest();

		// Yを入力方向に向ける
		rotDest.y = atan2f(-input.moveDir.x, -input.moveDir.z);

		// 目標の向きに設定する
		SetRotDest(rotDest);
	}

	// 位置の取得
	D3DXVECTOR3 pos = GetPos();

	if (pos.y < RESPAWN_HEIGHT)
	{
		// リスポーン処理
		Respawn(Respawn::POS);
	}

	if (m_pShadowS != nullptr)
	{
		// ステンシルシャドウの位置設定
		m_pShadowS->SetPosition(GetPos());
	}

	// モーションの更新処理
	m_pMotion->Update(m_apModel, m_nNumModel);
}
//=============================================================================
// 描画処理
//=============================================================================
void CPlayer::Draw(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 計算用マトリックス
	D3DXMATRIX mtxRot, mtxTrans, mtxSize;

	// ワールドマトリックスの初期化
	D3DXMatrixIdentity(&m_mtxWorld);

	// サイズを反映
	D3DXMatrixScaling(&mtxSize, GetSize().x, GetSize().y, GetSize().z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxSize);

	// 向きを反映
	D3DXMatrixRotationYawPitchRoll(&mtxRot, GetRot().y, GetRot().x, GetRot().z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxRot);

	// 位置を反映
	D3DXMatrixTranslation(&mtxTrans, GetPos().x, GetPos().y, GetPos().z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxTrans);

	// ワールドマトリックスを設定
	pDevice->SetTransform(D3DTS_WORLD, &m_mtxWorld);

	for (int nCntMat = 0; nCntMat < m_nNumModel; nCntMat++)
	{
		// モデル(パーツ)の描画
		if (m_apModel[nCntMat])
		{
			m_apModel[nCntMat]->Draw();
		}
	}

#ifdef _DEBUG

	btRigidBody* pRigid = GetRigidBody();
	btCollisionShape* pShape = GetCollisionShape();

	// カプセルコライダーの描画
	if (pRigid && pShape)
	{
		btTransform transform;
		pRigid->getMotionState()->getWorldTransform(transform);

		m_pDebug3D->DrawCapsuleCollider((btCapsuleShape*)pShape, transform, INIT_XCOL_WHITE);
	}

#endif

}
//=============================================================================
// リスポーン(直接設定)処理
//=============================================================================
void CPlayer::Respawn(D3DXVECTOR3 pos)
{
	D3DXVECTOR3 respawnPos = pos; // 任意の位置

	GetPos() = respawnPos;

	btRigidBody* pRigid = GetRigidBody();

	if (pRigid)
	{
		pRigid->setLinearVelocity(btVector3(0, 0, 0));
		pRigid->setAngularVelocity(btVector3(0, 0, 0));

		// ワールド座標更新
		btTransform trans;
		trans.setIdentity();
		trans.setOrigin(btVector3(respawnPos.x, respawnPos.y, respawnPos.z));

		pRigid->setWorldTransform(trans);

		if (pRigid->getMotionState())
		{
			pRigid->getMotionState()->setWorldTransform(trans);
		}
	}
}
//=============================================================================
// プレイヤーの前方ベクトル取得
//=============================================================================
D3DXVECTOR3 CPlayer::GetForward(void)
{
	D3DXVECTOR3 forward(-m_mtxWorld._31, m_mtxWorld._32, -m_mtxWorld._33);

	// 正規化する
	D3DXVec3Normalize(&forward, &forward);

	return forward;
}
//=============================================================================
// 入力判定取得関数
//=============================================================================
CPlayer::InputData CPlayer::GatherInput(void)
{
	InputData input{};
	input.moveDir = INIT_VEC3;

	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();	// キーボードの取得
	CInputJoypad* pJoypad = CManager::GetInputJoypad();			// ジョイパッドの取得
	XINPUT_STATE* pStick = pJoypad->GetStickAngle();			// スティックの取得
	CCamera* pCamera = CManager::GetCamera();					// カメラの取得
	D3DXVECTOR3 CamRot = pCamera->GetRot();						// カメラ角度の取得

	if (this == nullptr || !m_canControl)
	{
		return input;
	}

	//---------------------------
	// インタラクト入力
	//---------------------------
	if (pKeyboard->GetPress(DIK_LCONTROL) || pJoypad->GetPressR2())
	{
		//input.stealth = true;
	}


	//---------------------------
	// ゲームパッド入力(左スティック)
	//---------------------------
	if (pJoypad->GetStick() && pStick)
	{
		// 左スティックの取得
		float stickX = pStick->Gamepad.sThumbLX;
		float stickY = pStick->Gamepad.sThumbLY;
		float magnitude = sqrtf(stickX * stickX + stickY * stickY);

		if (magnitude >= CInputJoypad::DEADZONE)
		{
			stickX /= magnitude;
			stickY /= magnitude;

			float normMag = 
				std::min((magnitude - CInputJoypad::DEADZONE) / (CInputJoypad::LSTICK_VALUE - CInputJoypad::DEADZONE), 1.0f);

			stickX *= normMag;
			stickY *= normMag;

			D3DXVECTOR3 dir;
			float yaw = CamRot.y;

			dir.x = -(stickX * cosf(yaw) + stickY * sinf(yaw));
			dir.z = stickX * sinf(-yaw) + stickY * cosf(yaw);
			dir.z = -dir.z;

			input.moveDir += D3DXVECTOR3(dir.x, 0, dir.z);
		}
	}

	//---------------------------
	// キーボード入力
	//---------------------------
	if (pKeyboard->GetPress(DIK_W))
	{
		input.moveDir += D3DXVECTOR3(-sinf(CamRot.y), 0, -cosf(CamRot.y));
	}
	if (pKeyboard->GetPress(DIK_S))
	{
		input.moveDir += D3DXVECTOR3(sinf(CamRot.y), 0, cosf(CamRot.y));
	}
	if (pKeyboard->GetPress(DIK_A))
	{
		input.moveDir += D3DXVECTOR3(cosf(CamRot.y), 0, -sinf(CamRot.y));
	}
	if (pKeyboard->GetPress(DIK_D))
	{
		input.moveDir += D3DXVECTOR3(-cosf(CamRot.y), 0, sinf(CamRot.y));
	}

	// 正規化
	if (input.moveDir.x != 0.0f || input.moveDir.z != 0.0f)
	{
		D3DXVec3Normalize(&input.moveDir, &input.moveDir);
	}

	return input;
}
////=============================================================================
//// ダメージ処理
////=============================================================================
//void CPlayer::Damage(float fDamage)
//{
//	if (!m_pMotion->IsCurrentMotion(DAMAGE))
//	{
//		// まず共通のHP処理
//		CCharacter::Damage(fDamage);
//
//		// ダメージステートへ
//		m_stateMachine.ChangeState<CPlayerDamageState>();
//	}
//}
//=============================================================================
// 減速処理
//=============================================================================
void CPlayer::ApplyDeceleration(void)
{
	D3DXVECTOR3 move = GetMove();

	// 減速させる
	move *= DECELERATION_RATE;

	if (fabsf(move.x) < 0.01f) move.x = 0;
	if (fabsf(move.z) < 0.01f) move.z = 0;

	// 移動量の設定
	SetMove(move);
}
//=============================================================================
// ステート用のフラグ更新処理
//=============================================================================
void CPlayer::UpdateMovementFlags(const D3DXVECTOR3& moveDir)
{
	m_bIsMoving = (moveDir.x != 0.0f || moveDir.z != 0.0f);
}
