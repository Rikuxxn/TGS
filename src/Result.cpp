//=============================================================================
//
// リザルト処理 [Result.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Result.h"
#include "Input.h"
#include "Manager.h"
#include "Ui.h"
#include "Light.h"
#include "BlockManager.h"
#include "Parameter.h"
#include "SkyCube.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************




//*****************************************************************************
// 名前空間
//*****************************************************************************
namespace ResultWorld
{
	const D3DXVECTOR3	PLAYER_POS{ 0.0f, 110.0f, 0.0f };
	const D3DXVECTOR3	PLAYER_ROT{ 0.0f, 180.0f, 0.0f };

}

namespace ResultCamera
{
	const CameraParam PARAM =
	{
		{64.0f, 170.8f, 50.6f},
		{-41.4f, 152.2f, -112.4f},
		{0.10f, 0.57f, 0.0f},
		0.0f
	};
}

namespace ResultLight
{
	const LightParam LIGHTS[] =
	{
		{ D3DLIGHT_DIRECTIONAL, {1.0f, 0.65f, 0.4f, 1.0f}, {-0.3f, -0.8f, 0.2f}, {0,300,0} },
		{ D3DLIGHT_DIRECTIONAL, {1.0f, 0.65f, 0.4f, 1.0f}, {0,-0.8f,-0.2f},   {0,300,0} },
		{ D3DLIGHT_DIRECTIONAL, {0.3f,0.35f,0.5f,1.0f}, {0,1,0},          {0,0,0}   },
		{ D3DLIGHT_DIRECTIONAL, {0.5f,0.4f,0.4f,1.0f},  {0.3f,-0.2f,-0.3f},{0,0,0}  }
	};
}

namespace ResultUI
{
	// 位置
	namespace Pos
	{

	}

	// 大きさ(幅、高さ)
	namespace Size
	{


	}

	// 色
	const D3DXCOLOR COLOR = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

//=============================================================================
// コンストラクタ
//=============================================================================
CResult::CResult() : CScene(CScene::MODE_RESULT)
{
	// 値のクリア
	m_pBlockManager = nullptr;	// ブロックマネージャーへのポインタ
	m_pLight		= nullptr;	// ライトへのポインタ
}
//=============================================================================
// デストラクタ
//=============================================================================
CResult::~CResult()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CResult::Init(void)
{
	// 名前空間リザルトルUIの使用
	using namespace ResultUI;

	// マウスカーソルを表示する
	CManager::GetInputMouse()->SetCursorVisibility(true);

	// ブロックマネージャーの生成
	m_pBlockManager = new CBlockManager;

	// ブロックマネージャーの初期化
	m_pBlockManager->Init();

	// ライトの生成
	m_pLight = new CLight;

	// ライトの初期化
	m_pLight->Init();

	// ライトの再設定処理
	ResetLight();

	// スカイキューブの生成
	CSkyCube::Create();

	//// JSONの読み込み
	//m_pBlockManager->LoadFromJson("data/StageInfo/Result_BlockInfo.json");

	//// UI生成
	//auto testUI = CUITexture::Create("data/TEXTURE/test.png", 
	//	Pos::TREASURE_COUNT_X, Pos::TREASURE_COUNT_Y, ResultUI::COLOR, Size::TREASURE_COUNT_W, Size::TREASURE_COUNT_H);

	//// UI登録
	//CUIManager::GetInstance()->AddUI("TEST", testUI);

	//// UIの初期設定
	//testUI->Hide();

	// カメラの取得
	CCamera* pCamera = CManager::GetCamera();

	// カメラの初期位置を設定しておく
	pCamera->SetCamParameter(
		ResultCamera::PARAM.eye,
		ResultCamera::PARAM.at,
		ResultCamera::PARAM.rot,
		ResultCamera::PARAM.distance);

	//// 音の取得
	//CSound* pSound = CManager::GetSound();

	//// リザルトSEの再生
	//if (pSound)
	//{
	//	pSound->Play(CSound::SOUND_LABEL_RESULTSE);
	//}

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CResult::Uninit(void)
{
	// ブロックマネージャーの破棄
	if (m_pBlockManager != nullptr)
	{
		m_pBlockManager->Uninit();

		delete m_pBlockManager;
		m_pBlockManager = nullptr;
	}

	// ライトの破棄
	if (m_pLight != nullptr)
	{
		delete m_pLight;
		m_pLight = nullptr;
	}
}
//=============================================================================
// 更新処理
//=============================================================================
void CResult::Update(void)
{
	// ブロックマネージャーの更新処理
	m_pBlockManager->Update();

	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();
	CInputJoypad* pJoypad = CManager::GetInputJoypad();		// ゲームパッドの入力取得
	CFade* pFade = CManager::GetFade();

	if (pFade->GetFade() == CFade::FADE_NONE && 
		(pKeyboard->GetAnyKeyTrigger() || pJoypad->GetTrigger(CInputJoypad::JOYKEY_A) ||
			CManager::GetInputKeyboard()->GetTrigger(DIK_RETURN)))
	{
		//// ランキング画面に移行
		//pFade->SetFade(MODE_RANKING);

		// タイトル画面に移行
		pFade->SetFade(MODE_TITLE);
	}

#ifdef _DEBUG
	if (pFade->GetFade() == CFade::FADE_NONE && pKeyboard->GetAnyKeyTrigger())
	{
		//// ランキング画面に移行
		//pFade->SetFade(MODE_RANKING);

		// タイトル画面に移行
		pFade->SetFade(MODE_TITLE);
	}
#endif
}
//=============================================================================
// 描画処理
//=============================================================================
void CResult::Draw(void)
{
	// カメラの取得
	CCamera* pCamera = CManager::GetCamera();

	if (!pCamera->GetMode() == CCamera::MODE_EDIT)
	{
		// カメラの設定
		pCamera->SetCamParameter(
			ResultCamera::PARAM.eye,
			ResultCamera::PARAM.at,
			ResultCamera::PARAM.rot,
			ResultCamera::PARAM.distance);
	}
}
//=============================================================================
// ライト設定処理
//=============================================================================
void CResult::ResetLight(void)
{
	// ライトを削除しておく
	CLight::Uninit();

	// ライトの設定
	for (const auto& light : ResultLight::LIGHTS)
	{
		CLight::AddLight(light.type, light.color, light.dir, light.pos);
	}
}
//=============================================================================
// デバイスリセット通知
//=============================================================================
void CResult::OnDeviceReset(void)
{
	// ライトの再設定処理
	ResetLight();
}
//=============================================================================
// サムネイルリリース通知
//=============================================================================
void CResult::ReleaseThumbnail(void)
{
	m_pBlockManager->ReleaseThumbnailRenderTarget();
}
//=============================================================================
// サムネイルリセット通知
//=============================================================================
void CResult::ResetThumbnail(void)
{
	m_pBlockManager->InitThumbnailRenderTarget(CManager::GetRenderer()->GetDevice());
	m_pBlockManager->GenerateThumbnailsForResources(); // キャッシュも再作成
}
