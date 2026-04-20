//=============================================================================
//
// ブロックマネージャー処理 [BlockManager.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "BlockManager.h"
#include "json.hpp"
#include "FileDialogUtils.h"
#include "Manager.h"
#include "imgui_internal.h"
#include "RayCast.h"
#include "Game.h"
#include "BlockList.h"
#include "Player.h"
#include "Tutorial.h"
#include "CharacterManager.h"
#include "Parameter.h"
#include "MathConst.h"

// JSONの使用
using json = nlohmann::json;

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
std::vector<CBlock*> CBlockManager::m_blocks = {};	// ブロックの情報
std::unordered_map<CBlock::TYPE, std::vector<CBlock*>> CBlockManager::m_blocksByType;
int CBlockManager::m_selectedIdx = 0;				// 選択中のインデックス
CBlock* CBlockManager::m_draggingBlock = {};		// ドラッグ中のブロック
std::unordered_map<CBlock::TYPE, std::string> CBlockManager::s_FilePathMap; 
CBlock* CBlockManager::m_selectedBlock = {};// 選択したブロック


namespace ThumbnailCamera
{
	const D3DXVECTOR3 EYE{ -120.0f, 100.0f, -120.0f };
	const D3DXVECTOR3 AT{ 0.0f, 0.0f, 0.0f };
	const D3DXVECTOR3 UP{ 0.0f, 1.0f, 0.0f };

	constexpr float FOV					= 60.0f;
	constexpr float MIN_RENDER_DISTANCE = 1.0f;
	constexpr float MAX_RENDER_DISTANCE = 1000.0f;
}


namespace ThumbnailLight
{
	const LightParam LIGHTS[] =
	{
		{ D3DLIGHT_DIRECTIONAL, {0.9f, 0.9f, 0.9f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0,300,0} },
		{ D3DLIGHT_DIRECTIONAL, {0.3f, 0.3f, 0.3f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0,300,0} },
		{ D3DLIGHT_DIRECTIONAL, {0.7f, 0.7f, 0.7f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0,0,0}   },
		{ D3DLIGHT_DIRECTIONAL, {0.7f, 0.7f, 0.7f, 1.0f}, {-1.0f, 0.0f, 0.0f},{0,0,0}  },
		{ D3DLIGHT_DIRECTIONAL, {0.7f, 0.7f, 0.7f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0,0,0}},
		{ D3DLIGHT_DIRECTIONAL, {0.7f, 0.7f, 0.7f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0,0,0}}
	};							 
}

//=============================================================================
// コンストラクタ
//=============================================================================
CBlockManager::CBlockManager()
{
	// 値のクリア
	m_selectedBlock				= nullptr;		// 選択中のブロック
	m_prevSelectedIdx			= -1;			// 前回の選択中のインデックス
	m_pDebug3D					= nullptr;		// 3Dデバッグ表示へのポインタ
	m_autoUpdateColliderSize	= true;			// コライダー自動更新フラグ
	m_isDragging				= false;		// ドラッグ中かどうか
	m_thumbWidth				= THUMB_WIDTH;	// サムネイルの幅
	m_thumbHeight				= THUMB_HEIGHT;	// サムネイルの高さ
}
//=============================================================================
// デストラクタ
//=============================================================================
CBlockManager::~CBlockManager()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CBlock* CBlockManager::CreateBlock(CBlock::TYPE type, D3DXVECTOR3 pos)
{
	const char* path = CBlockManager::GetFilePathFromType(type);

	CBlock* newBlock = CBlock::Create(path, pos, INIT_VEC3, VEC3_DEFAULT, type);

	if (newBlock)
	{
		// 全体のリストに追加
		m_blocks.push_back(newBlock);

		// タイプ別キャッシュにも追加
		m_blocksByType[type].push_back(newBlock);
	}

	return newBlock;
}
//=============================================================================
// 初期化処理
//=============================================================================
void CBlockManager::Init(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	InitThumbnailRenderTarget(pDevice);

	LoadConfig("data/model_list.json");

	// サムネイルキャッシュ作成
	GenerateThumbnailsForResources();

	// 動的配列を空にする (サイズを0にする)
	m_blocks.clear();
	m_blocksByType.clear();
}
//=============================================================================
// サムネイルのレンダーターゲットの初期化
//=============================================================================
HRESULT CBlockManager::InitThumbnailRenderTarget(LPDIRECT3DDEVICE9 device)
{
	if (!device)
	{
		return E_FAIL;
	}

	// サムネイル用レンダーターゲットテクスチャの作成
	if (FAILED(device->CreateTexture(
		(UINT)m_thumbWidth, (UINT)m_thumbHeight,
		1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&m_pThumbnailRT, nullptr)))
	{
		return E_FAIL;
	}

	// サムネイル用深度ステンシルサーフェスの作成
	if (FAILED(device->CreateDepthStencilSurface(
		(UINT)m_thumbWidth, (UINT)m_thumbHeight,
		D3DFMT_D24S8,
		D3DMULTISAMPLE_NONE, 0, TRUE,
		&m_pThumbnailZ, nullptr)))
	{
		return E_FAIL;
	}

	return S_OK;
}
//=============================================================================
// サムネイルのレンダーターゲットの設定
//=============================================================================
IDirect3DTexture9* CBlockManager::RenderThumbnail(CBlock* pBlock)
{
	if (!pBlock || !m_pThumbnailRT || !m_pThumbnailZ)
	{
		return nullptr;
	}

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	if (!pDevice)
	{
		return nullptr;
	}

	// 名前空間ThumbnailCameraの使用
	using namespace ThumbnailCamera;

	// サムネイル描画用の新規テクスチャ作成
	IDirect3DTexture9* pTex = nullptr;
	if (FAILED(pDevice->CreateTexture(
		(UINT)m_thumbWidth, (UINT)m_thumbHeight, 1,
		D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT, &pTex, nullptr)))
	{
		return nullptr;
	}

	// 現在のレンダーターゲットと深度バッファを保存
	LPDIRECT3DSURFACE9 pOldRT = nullptr;
	LPDIRECT3DSURFACE9 pOldZ = nullptr;
	LPDIRECT3DSURFACE9 pNewRT = nullptr;

	pDevice->GetRenderTarget(0, &pOldRT);
	pDevice->GetDepthStencilSurface(&pOldZ);
	pTex->GetSurfaceLevel(0, &pNewRT);

	// サムネイル用のレンダーターゲットに切り替え
	pDevice->SetRenderTarget(0, pNewRT);
	pDevice->SetDepthStencilSurface(m_pThumbnailZ);

	// クリア
	pDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(50, 50, 50), 1.0f, 0);
	pDevice->BeginScene();

	// 固定カメラ
	D3DXMATRIX matView, matProj;
	D3DXMatrixLookAtLH(&matView, &EYE, &AT, &UP);

	// プロジェクションマトリックスの作成
	D3DXMatrixPerspectiveFovLH(&matProj,
		D3DXToRadian(FOV),							// 視野角
		m_thumbWidth / m_thumbHeight,				// アスペクト比
		MIN_RENDER_DISTANCE,						// 近クリップ面
		MAX_RENDER_DISTANCE);						// 遠クリップ面

	pDevice->SetTransform(D3DTS_VIEW, &matView);
	pDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	// ライトのバックアップ
	auto backup = CLight::GetCurrentLights();

	// 既存のライトを無効化
	CLight::Uninit();

	// ライトの設定
	for (const auto& light : ThumbnailLight::LIGHTS)
	{
		CLight::AddLight(light.type, light.color, light.dir, light.pos);
	}

	// サムネイル用モデル描画
	pBlock->Draw();

	// 元のライトを復元
	CLight::RestoreLights(backup);

	// 描画終了
	pDevice->EndScene();

	// 元のレンダーターゲットと深度バッファに戻す
	pDevice->SetRenderTarget(0, pOldRT);
	pDevice->SetDepthStencilSurface(pOldZ);

	// 破棄
	if (pOldRT) pOldRT->Release();
	if (pOldZ)  pOldZ->Release();
	if (pNewRT) pNewRT->Release();

	return pTex;
}
//=============================================================================
// サムネイル用のモデル生成処理
//=============================================================================
void CBlockManager::GenerateThumbnailsForResources(void)
{
	// 既存サムネイルを解放
	for (auto tex : m_thumbnailTextures)
	{
		if (tex)
		{
			tex->Release();
		}
	}

	m_thumbnailTextures.clear();
	m_thumbnailTextures.resize((size_t)CBlock::TYPE_MAX, nullptr);

	for (size_t nCnt = 0; nCnt < (int)CBlock::TYPE_MAX; ++nCnt)
	{
		// 一時ブロック生成（位置は原点）
		CBlock::TYPE payloadType = static_cast<CBlock::TYPE>(nCnt);
		CBlock* pTemp = CreateBlock(payloadType, D3DXVECTOR3(0, 0, 0));
		if (!pTemp)
		{
			continue;
		}

		if (!m_thumbnailTextures[nCnt])
		{
			// サムネイル作成
			m_thumbnailTextures[nCnt] = RenderThumbnail(pTemp);
		}

		pTemp->Kill();                 // 削除フラグを立てる
		CleanupDeadBlocks();           // 配列から取り除き、メモリ解放
	}
}
//=============================================================================
// サムネイルテクスチャの取得
//=============================================================================
IDirect3DTexture9* CBlockManager::GetThumbnailTexture(size_t index)
{
	assert(index < m_thumbnailTextures.size());
	return m_thumbnailTextures[index];
}
//=============================================================================
// サムネイルの破棄
//=============================================================================
void CBlockManager::ReleaseThumbnailRenderTarget(void)
{
	// レンダーターゲットの破棄
	if (m_pThumbnailRT)
	{
		m_pThumbnailRT->Release();
		m_pThumbnailRT = nullptr;
	}

	if (m_pThumbnailZ)
	{
		m_pThumbnailZ->Release();
		m_pThumbnailZ = nullptr;
	}

	// サムネイルキャッシュも解放しておく
	for (auto& tex : m_thumbnailTextures)
	{
		if (tex)
		{
			tex->Release();
			tex = nullptr;
		}
	}
	m_thumbnailTextures.clear();
	m_thumbnailsGenerated = false;
}
//=============================================================================
// 終了処理
//=============================================================================
void CBlockManager::Uninit(void)
{
	// サムネイルの破棄
	ReleaseThumbnailRenderTarget();

	// 動的配列を空にする (サイズを0にする)
	m_blocks.clear();
	m_blocksByType.clear();
}
//=============================================================================
// 削除予約があるブロックの削除処理
//=============================================================================
void CBlockManager::CleanupDeadBlocks(void)
{
	for (int nCnt = (int)m_blocks.size() - 1; nCnt >= 0; nCnt--)
	{
		CBlock* pBlock = m_blocks[nCnt];

		if (m_blocks[nCnt]->IsDead())
		{
			// m_blocksByTypeからも削除
			auto& list = m_blocksByType[pBlock->GetType()];
			list.erase(std::remove(list.begin(), list.end(), pBlock), list.end());

			// ブロックの終了処理
			m_blocks[nCnt]->Uninit();
			m_blocks.erase(m_blocks.begin() + nCnt);
		}
	}
}
//=============================================================================
// 更新処理
//=============================================================================
void CBlockManager::Update(void)
{

#ifdef _DEBUG

	// 情報の更新
	UpdateInfo();

#endif
	// ブロック削除処理
	CleanupDeadBlocks();
}
//=============================================================================
// 描画処理
//=============================================================================
void CBlockManager::Draw(void)
{
#ifdef _DEBUG
	//// 選択中のブロックだけコライダー描画
	//CBlock* pSelectBlock = GetSelectedBlock();
	//if (pSelectBlock != nullptr)
	//{
	//	pSelectBlock->DrawCollider();
	//}
#endif
}
//=============================================================================
// 情報の更新処理
//=============================================================================
void CBlockManager::UpdateInfo(void)
{
	// GUIスタイルの取得
	ImGuiStyle& style = ImGui::GetStyle();

	style.GrabRounding		= 10.0f;		// スライダーのつまみを丸く
	style.ScrollbarRounding = 10.0f;		// スクロールバーの角
	style.ChildRounding		= 10.0f;		// 子ウィンドウの角
	style.WindowRounding	= 10.0f;		// ウィンドウ全体の角

	// 場所
	CImGuiManager::Instance().SetPosImgui(ImVec2(1280.0f, 20.0f));

	// サイズ
	CImGuiManager::Instance().SetSizeImgui(ImVec2(420.0f, 500.0f));

	// 最初のGUI
	CImGuiManager::Instance().StartImgui("BlockInfo", CImGuiManager::IMGUITYPE_DEFOULT);

	// ブロックがない場合
	if (m_blocks.empty())
	{
		ImGui::Text("No blocks placed yet.");
	}
	else
	{
		// ブロックの総数
		ImGui::Text("Block Num %d", (int)m_blocks.size());

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		// インデックス選択
		ImGui::SliderInt("Block Index", &m_selectedIdx, 0, (int)m_blocks.size() - 1);

		// 範囲外対策
		if (m_selectedIdx >= (int)m_blocks.size())
		{
			m_selectedIdx = (int)m_blocks.size() - 1;
		}

		// 前回選んでたブロックを解除
		if (m_prevSelectedIdx != -1 && m_prevSelectedIdx != m_selectedIdx)
		{
			m_blocks[m_prevSelectedIdx]->SetSelected(false);
		}

		// 対象ブロックの取得
		m_selectedBlock = m_blocks[m_selectedIdx];

		// ブロック情報の調整処理
		UpdateTransform(m_selectedBlock);

		if (GetUpdateCollider() == false)
		{
			// コライダーの調整処理
			UpdateCollider(m_selectedBlock);
		}
		else
		{
			ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける
			ImGui::Text("Auto Update Collider Size is Active");
		}
	}

	ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

	// ブロックタイプ一覧
	if (ImGui::TreeNode("Block Types"))
	{
		ImGui::BeginChild("BlockTypeList", ImVec2(0, 500), true); // スクロール領域

		int numTypes = (int)CBlock::TYPE_MAX;

		for (int nCnt = 0; nCnt < numTypes; nCnt++)
		{
			IDirect3DTexture9* pThumb = GetThumbnailTexture(nCnt); // サムネイル取得

			if (!pThumb)
			{
				continue; // nullptr はスキップ
			}

			ImGui::PushID(nCnt);
			ImGui::Image(reinterpret_cast<ImTextureID>(pThumb), ImVec2(m_thumbWidth, m_thumbHeight));

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				m_isDragging = true;// ドラッグ中
				CBlock::TYPE payloadType = static_cast<CBlock::TYPE>(nCnt);
				ImGui::SetDragDropPayload("BLOCK_TYPE", &payloadType, sizeof(payloadType));
				ImGui::Text("Block Type %d", nCnt);
				ImGui::Image(reinterpret_cast<ImTextureID>(pThumb), ImVec2(m_thumbWidth, m_thumbHeight));
				ImGui::EndDragDropSource();
			}

			// マウスの取得
			CInputMouse* pMouse = CManager::GetInputMouse();

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && m_isDragging)
			{
				// 現在のImGuiの内部状態（コンテキスト）へのポインターを取得
				ImGuiContext* ctx = ImGui::GetCurrentContext();

				if (ctx->DragDropPayload.IsDataType("BLOCK_TYPE"))
				{
					// ドラッグ中を解除
					m_isDragging = false;

					CBlock::TYPE draggedType = *(CBlock::TYPE*)ctx->DragDropPayload.Data;

					D3DXVECTOR3 pos = pMouse->GetGroundHitPosition();
					pos.y = 30.0f;

					// ブロックの生成
					m_draggingBlock = CreateBlock(draggedType, pos);
				}
			}

			ImGui::PopID();

			ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 画像間の余白
		}

		ImGui::EndChild();
		ImGui::TreePop();
	}

	ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

	if (ImGui::Button("Save"))
	{
		// ダイアログを開いてファイルに保存
		std::string path = OpenWindowsSaveFileDialog();

		if (!path.empty())
		{
			// データの保存
			CBlockManager::SaveToJson(path.c_str());
		}
	}

	ImGui::SameLine(0);

	if (ImGui::Button("Load"))
	{
		// ダイアログを開いてファイルを開く
		std::string path = OpenWindowsOpenFileDialog();

		if (!path.empty())
		{
			// データの読み込み
			CBlockManager::LoadFromJson(path.c_str());
		}
	}

	ImGui::End();

	//if (CManager::GetMode() == MODE_TITLE)
	//{
	//	return;
	//}

	// マウス選択処理
	PickBlockFromMouseClick();
}
//=============================================================================
// ブロック情報の調整処理
//=============================================================================
void CBlockManager::UpdateTransform(CBlock* selectedBlock)
{
	if (selectedBlock)
	{
		// 選択中のブロックの色を変える
		selectedBlock->SetSelected(true);

		D3DXVECTOR3 pos = selectedBlock->GetPos();	// 選択中のブロックの位置の取得
		D3DXVECTOR3 rot = selectedBlock->GetRot();	// 選択中のブロックの向きの取得
		D3DXVECTOR3 size = selectedBlock->GetSize();// 選択中のブロックのサイズの取得
		btScalar mass = selectedBlock->GetMass();	// 選択中のブロックの質量の取得

		// ラジアン→角度に一時変換（静的変数で保持し操作中のみ更新）
		static D3DXVECTOR3 degRot = D3DXToDegree(rot);
		static bool m_initializedDegRot = false;

		if (!m_initializedDegRot)
		{
			degRot = D3DXToDegree(rot);
			m_initializedDegRot = true;
		}

		bool isEditMode = selectedBlock->IsEditMode();

		ImGui::Checkbox("Kinematic", &isEditMode);

		if (isEditMode)
		{
			selectedBlock->SetEditMode(true);  // チェックでKinematic化
		}
		else
		{
			selectedBlock->SetEditMode(false); // 通常に戻す
		}

		//*********************************************************************
		// POS の調整
		//*********************************************************************

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		// ラベル
		ImGui::Text("POS"); ImGui::SameLine(60); // ラベルの位置ちょっと調整

		// X
		ImGui::Text("X:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Block_pos_x", &pos.x, 1.0f, -9000.0f, 9000.0f, "%.1f");

		// Y
		ImGui::SameLine();
		ImGui::Text("Y:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Block_pos_y", &pos.y, 1.0f, -9000.0f, 9000.0f, "%.1f");

		// Z
		ImGui::SameLine();
		ImGui::Text("Z:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Block_pos_z", &pos.z, 1.0f, -9000.0f, 9000.0f, "%.1f");

		//*********************************************************************
		// ROT の調整
		//*********************************************************************

		ImGui::Dummy(ImVec2(0.0f, 10.0f));

		ImGui::Text("ROT"); ImGui::SameLine(60);

		ImGui::Text("X:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		bool changedX = ImGui::DragFloat("##Block_rot_x", &degRot.x, 0.1f, -180.0f, 180.0f, "%.2f");

		ImGui::SameLine();
		ImGui::Text("Y:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		bool changedY = ImGui::DragFloat("##Block_rot_y", &degRot.y, 0.1f, -180.0f, 180.0f, "%.2f");

		ImGui::SameLine();
		ImGui::Text("Z:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		bool changedZ = ImGui::DragFloat("##Block_rot_z", &degRot.z, 0.1f, -180.0f, 180.0f, "%.2f");

		bool isRotChanged = changedX || changedY || changedZ;
		
		//*********************************************************************
		// SIZE の調整
		//*********************************************************************

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		// ラベル
		ImGui::Text("SIZE"); ImGui::SameLine(60); // ラベルの位置ちょっと調整

		// X
		ImGui::Text("X:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Block_size_x", &size.x, 0.1f, -100.0f, 100.0f, "%.1f");

		// Y
		ImGui::SameLine();
		ImGui::Text("Y:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Block_size_y", &size.y, 0.1f, -100.0f, 100.0f, "%.1f");

		// Z
		ImGui::SameLine();
		ImGui::Text("Z:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Block_size_z", &size.z, 0.1f, -100.0f, 100.0f, "%.1f");

		// チェックボックス：拡大率に応じて自動更新するか
		ImGui::Checkbox("Auto Update Collider Size", &m_autoUpdateColliderSize);

		// 前回のサイズを保持
		static D3DXVECTOR3 prevSize = selectedBlock->GetSize();

		// サイズ変更チェック
		bool isSizeChanged = (size != prevSize);

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		// ブロックの特殊処理
		selectedBlock->DrawCustomUI();

		//*********************************************************************
		// 質量 の調整
		//*********************************************************************

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		// ラベル
		ImGui::Text("MASS"); ImGui::SameLine(60); // ラベルの位置ちょっと調整

		// スライダー（範囲: 0.0f ～ 80.0f）

		float Mass = (float)mass;

		ImGui::SliderFloat("##MassSlider", &Mass, 0.0f, 80.0f, "%.2f");


		// 角度→ラジアンに戻す
		D3DXVECTOR3 rotRad = D3DXToRadian(degRot);

		// 位置の設定
		selectedBlock->SetPos(pos);

		// サイズの設定
		selectedBlock->SetSize(size);

		// サイズ(拡大率)が変わったときだけ呼ぶ
		if (m_autoUpdateColliderSize == true && isSizeChanged)
		{
			// ブロックサイズによるコライダーの生成
			selectedBlock->CreatePhysicsFromScale(size);

			prevSize = size; // 更新
		}

		if (isRotChanged)
		{
			// 回転が変わった時だけセット
			selectedBlock->SetRot(rotRad);

			// 編集モードなら即物理Transformも更新して同期
			if (isEditMode)
			{
				btTransform transform;
				transform.setIdentity();

				btVector3 btPos(pos.x + selectedBlock->GetColliderOffset().x,
					pos.y + selectedBlock->GetColliderOffset().y,
					pos.z + selectedBlock->GetColliderOffset().z);
				transform.setOrigin(btPos);

				D3DXMATRIX matRot;
				D3DXMatrixRotationYawPitchRoll(&matRot, rotRad.y, rotRad.x, rotRad.z);

				D3DXQUATERNION dq;
				D3DXQuaternionRotationMatrix(&dq, &matRot);

				btQuaternion btRot(dq.x, dq.y, dq.z, dq.w);
				transform.setRotation(btRot);

				if (!selectedBlock->GetRigidBody())
				{
					return;
				}
				selectedBlock->GetRigidBody()->setWorldTransform(transform);
				selectedBlock->GetRigidBody()->getMotionState()->setWorldTransform(transform);
			}
		}
		else
		{
			// 編集していない時はdegRotをselectedBlockの値に同期しておく
			degRot = D3DXToDegree(selectedBlock->GetRot());
		}

		//*********************************************************************
		// ブロックの削除
		//*********************************************************************

		ImGui::Dummy(ImVec2(0.0f, 10.0f));

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));

		if (ImGui::Button("Delete"))
		{
			if (m_autoUpdateColliderSize)
			{
				if (m_blocks[m_selectedIdx])
				{
					// 選択中のブロックを削除
					m_blocks[m_selectedIdx]->Uninit();
				}

				m_blocks.erase(m_blocks.begin() + m_selectedIdx);

				// 選択インデックスを調整
				if (m_selectedIdx >= (int)m_blocks.size())
				{
					m_selectedIdx = (int)m_blocks.size() - 1;
				}

				m_prevSelectedIdx = -1;
			}
			else
			{// m_autoUpdateColliderSizeがfalseの時は何もしない
				
			}
		}

		ImGui::PopStyleColor(3);
	}

	// 最後に保存
	m_prevSelectedIdx = m_selectedIdx;
}
//=============================================================================
// ブロック選択処理
//=============================================================================
void CBlockManager::PickBlockFromMouseClick(void)
{
	// ImGuiがマウスを使ってるなら選択処理をキャンセル
	if (ImGui::GetIO().WantCaptureMouse)
	{
		return;
	}

	// 左クリックのみ
	if (!CManager::GetInputMouse()->GetTrigger(0))
	{
		return;
	}

	// レイ取得（CRayCastを使用）
	D3DXVECTOR3 rayOrigin, rayDir;
	CRayCast::GetMouseRay(rayOrigin, rayDir);

	float minDist = FLT_MAX;
	int hitIndex = -1;

	for (size_t nCnt = 0; nCnt < m_blocks.size(); nCnt++)
	{
		CBlock* block = m_blocks[nCnt];

		// ワールド行列の取得（位置・回転・拡大を含む）
		D3DXMATRIX world = block->GetWorldMatrix();

		D3DXVECTOR3 modelSize = block->GetModelSize();
		D3DXVECTOR3 scale = block->GetSize();

		D3DXVECTOR3 halfSize;
		halfSize.x = modelSize.x * CMathConstant::HALF;
		halfSize.y = modelSize.y * CMathConstant::HALF;
		halfSize.z = modelSize.z * CMathConstant::HALF;

		float dist = 0.0f;
		if (CRayCast::IntersectOBB(rayOrigin, rayDir, world, halfSize, dist))
		{
			if (dist < minDist)
			{
				minDist = dist;
				hitIndex = nCnt;
			}
		}
	}

	// 選択状態を反映
	if (hitIndex >= 0)
	{
		// 以前選ばれていたブロックを非選択に
		if (m_prevSelectedIdx != -1 && m_prevSelectedIdx != hitIndex)
		{
			m_blocks[m_prevSelectedIdx]->SetSelected(false);
		}

		// 新しく選択
		m_selectedIdx = hitIndex;
		m_blocks[m_selectedIdx]->SetSelected(true);
		m_prevSelectedIdx = hitIndex;
	}
}
//=============================================================================
// コライダーの調整処理
//=============================================================================
void CBlockManager::UpdateCollider(CBlock* selectedBlock)
{
	// 単一コライダー用
	D3DXVECTOR3 colliderSize = selectedBlock->GetColliderSize();
	static D3DXVECTOR3 prevSize = colliderSize;

	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	ImGui::Text("COLLIDER SIZE");

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	bool changed = false;

	ImGui::Text("X:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	changed |= ImGui::DragFloat("##collider_size_x", &colliderSize.x, 0.1f, 0.1f, 800.0f, "%.1f");

	ImGui::SameLine();
	ImGui::Text("Y:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	changed |= ImGui::DragFloat("##collider_size_y", &colliderSize.y, 0.1f, 0.1f, 800.0f, "%.1f");

	ImGui::SameLine();
	ImGui::Text("Z:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	changed |= ImGui::DragFloat("##collider_size_z", &colliderSize.z, 0.1f, 0.1f, 800.0f, "%.1f");

	if (changed && colliderSize != prevSize)
	{
		colliderSize.x = std::max(colliderSize.x, 0.01f);
		colliderSize.y = std::max(colliderSize.y, 0.01f);
		colliderSize.z = std::max(colliderSize.z, 0.01f);

		selectedBlock->SetColliderManual(colliderSize);

		prevSize = colliderSize;
	}

	D3DXVECTOR3 offset = selectedBlock->GetColliderOffset();
	static D3DXVECTOR3 prevOffset = offset;

	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	ImGui::Text("COLLIDER OFFSET");

	bool offsetChanged = false;

	ImGui::Text("X:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	offsetChanged |= ImGui::DragFloat("##collider_offset_x", &offset.x, 0.1f, -800.0f, 800.0f, "%.1f");

	ImGui::SameLine();
	ImGui::Text("Y:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	offsetChanged |= ImGui::DragFloat("##collider_offset_y", &offset.y, 0.1f, -800.0f, 800.0f, "%.1f");

	ImGui::SameLine();
	ImGui::Text("Z:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	offsetChanged |= ImGui::DragFloat("##collider_offset_z", &offset.z, 0.1f, -800.0f, 800.0f, "%.1f");

	if (offsetChanged && offset != prevOffset)
	{
		selectedBlock->SetColliderOffset(offset);
		selectedBlock->CreatePhysics(selectedBlock->GetPos(), selectedBlock->GetColliderSize());

		prevOffset = offset;
	}
}
//=============================================================================
// Xファイルパスの読み込み
//=============================================================================
void CBlockManager::LoadConfig(const std::string& filename)
{
	std::ifstream ifs(filename);
	if (!ifs)
	{
		return;
	}

	json j;
	ifs >> j;

	// j は配列になってるのでループする
	for (auto& block : j)
	{
		int typeInt = block["type"];
		std::string filepath = block["filepath"];

		s_FilePathMap[(CBlock::TYPE)typeInt] = filepath;
	}
}
//=============================================================================
// 特定のタイプのブロックを取得する処理
//=============================================================================
const std::vector<CBlock*>& CBlockManager::GetBlocksByType(CBlock::TYPE type)
{
	return m_blocksByType[type]; // 存在しない場合は空vectorが返る
}
//=============================================================================
// タイプからXファイルパスを取得
//=============================================================================
const char* CBlockManager::GetFilePathFromType(CBlock::TYPE type)
{
	auto it = s_FilePathMap.find(type);
	return (it != s_FilePathMap.end()) ? it->second.c_str() : "";
}
//=============================================================================
// ブロック情報の保存処理
//=============================================================================
void CBlockManager::SaveToJson(const char* filename)
{
	// JSONオブジェクト
	json j;

	// 1つづつJSON化
	for (const auto& block : m_blocks)
	{
		json b;
		block->SaveToJson(b);

		// 追加
		j.push_back(b);
	}

	// 出力ファイルストリーム
	std::ofstream file(filename);

	if (file.is_open())
	{
		file << std::setw(4) << j;

		// ファイルを閉じる
		file.close();
	}
}
//=============================================================================
// ブロック情報の読み込み処理
//=============================================================================
void CBlockManager::LoadFromJson(const char* filename)
{
	std::ifstream file(filename);

	if (!file.is_open())
	{// 開けなかった
		return;
	}

	json j;
	file >> j;

	// ファイルを閉じる
	file.close();

	// 既存のブロックを消す
	for (auto block : m_blocks)
	{
		if (block != nullptr)
		{
			// ブロックの終了処理
			block->Uninit();
		}
	}

	// 動的配列を空にする (サイズを0にする)
	m_blocks.clear();
	m_blocksByType.clear();

	// 新たに生成
	for (const auto& b : j)
	{
		CBlock::TYPE type = b["type"];
		D3DXVECTOR3 pos(b["pos"][0], b["pos"][1], b["pos"][2]);

		// ブロックの生成
		CBlock* block = CreateBlock(type, pos);

		if (!block)
		{
			continue;
		}

		block->LoadFromJson(b);
	}
}
//=============================================================================
// ライト更新処理
//=============================================================================
void CBlockManager::UpdateLight(void)
{
	for (const auto& block : m_blocks)
	{
		block->UpdateLight();
	}
}