//=============================================================================
//
// タイトル処理 [Title.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Title.h"
#include "Input.h"
#include "Manager.h"
#include "SkyCube.h"
#include "Parameter.h"
#include "MathConst.h"


//*****************************************************************************
// 名前空間
//*****************************************************************************
namespace TitleUI
{
	constexpr int	ITEM_NUM			= 1;			// タイトルのUI数
	constexpr float	ANCHOR_X			= 0.25f;		// 横位置（%）
	constexpr float	ANCHOR_Y			= 0.3f;			// 縦位置（%）
	constexpr float	LOGO_WIDTH_RATE		= 0.4f;			// 画面幅に対しての項目幅率
	constexpr float	LOGO_HEIGHT_RATE	= 0.3f;			// 画面高さに対しての項目高さ率
}

namespace TitleWorld
{
	const D3DXVECTOR3 PLAYER_POS{ 300.0f, 110.0f, 450.0f };
	const D3DXVECTOR3 PLAYER_ROT{ 0.0f, 90.0f, 0.0f };
}

namespace TitleCamera
{
	const CameraParam PARAM =
	{
		{203.5f, 120.5f,  551.5f},
		{902.5f, 292.0f, 380.0f},
		{-0.23f, -1.33f, 0.0f},
		0.0f
	};
}

namespace TitleLight
{
	const LightParam LIGHTS[] =
	{
		{ D3DLIGHT_DIRECTIONAL, {1.0f,0.65f,0.4f,1.0f}, {-0.3f,-0.8f,0.2f}, {0,300,0} },
		{ D3DLIGHT_DIRECTIONAL, {1.0f,0.65f,0.4f,1.0f}, {0,-0.8f,-0.2f},   {0,300,0} },
		{ D3DLIGHT_DIRECTIONAL, {0.3f,0.35f,0.5f,1.0f}, {0,1,0},          {0,0,0}   },
		{ D3DLIGHT_DIRECTIONAL, {0.5f,0.4f,0.4f,1.0f},  {0.3f,-0.2f,-0.3f},{0,0,0}  }
	};
}

namespace Particle
{
	const D3DXCOLOR COLOR{ 0.8f, 0.8f, 0.8f, 0.8f };
	constexpr int NUM = 1;
}

//=============================================================================
// コンストラクタ
//=============================================================================
CTitle::CTitle() : CScene(CScene::MODE_TITLE)
{
	// 値のクリア
	m_pVtxBuff			= nullptr;		// 頂点バッファへのポインタ
	m_nIdxTextureTitle	= 0;			// テクスチャインデックス
	m_pLight			= nullptr;		// ライトへのポインタ
	m_pBlockManager		= nullptr;		// ブロックマネージャーへのポインタ
	for (int nCnt = 0; nCnt < TYPE_MAX; nCnt++)
	{
		m_vertexRanges[nCnt] = { -1, -1 }; // 未使用値で初期化
	}
}
//=============================================================================
// デストラクタ
//=============================================================================
CTitle::~CTitle()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CTitle::Init(void)
{
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
	//m_pBlockManager->LoadFromJson("data/StageInfo/Title_BlockInfo.json");

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// テクスチャの取得
	m_nIdxTextureTitle = CManager::GetTexture()->RegisterDynamic("data/TEXTURE/title.png");

	m_vertexRanges[TYPE_FIRST] = { 0, 3 }; // タイトル

	// 頂点バッファの生成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * VERTEX * TitleUI::ITEM_NUM,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&m_pVtxBuff,
		NULL);

	VERTEX_2D* pVtx;// 頂点情報へのポインタ

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);
	
	// バックバッファサイズの取得
	float screenW = (float)CManager::GetRenderer()->GetBackBufferWidth();
	float screenH = (float)CManager::GetRenderer()->GetBackBufferHeight();

	float logoW = screenW * TitleUI::LOGO_WIDTH_RATE;
	float logoH = screenH * TitleUI::LOGO_HEIGHT_RATE;

	float centerX = screenW * TitleUI::ANCHOR_X;
	float centerY = screenH * TitleUI::ANCHOR_Y;

	ImageInfo images[TitleUI::ITEM_NUM] =
	{
		{ D3DXVECTOR3(centerX, centerY, 0.0f), logoW * CMathConstant::HALF, logoH * CMathConstant::HALF },
	};

	for (int nCnt = 0; nCnt < TitleUI::ITEM_NUM; nCnt++)
	{
		// 頂点座標の設定
		pVtx[0].pos = D3DXVECTOR3(images[nCnt].pos.x - images[nCnt].width, images[nCnt].pos.y - images[nCnt].height, 0.0f);
		pVtx[1].pos = D3DXVECTOR3(images[nCnt].pos.x + images[nCnt].width, images[nCnt].pos.y - images[nCnt].height, 0.0f);
		pVtx[2].pos = D3DXVECTOR3(images[nCnt].pos.x - images[nCnt].width, images[nCnt].pos.y + images[nCnt].height, 0.0f);
		pVtx[3].pos = D3DXVECTOR3(images[nCnt].pos.x + images[nCnt].width, images[nCnt].pos.y + images[nCnt].height, 0.0f);

		// rhwの設定
		pVtx[0].rhw = 
		pVtx[1].rhw = 
		pVtx[2].rhw = 
		pVtx[3].rhw = 1.0f;

		//頂点カラーの設定
		pVtx[0].col = 
		pVtx[1].col = 
		pVtx[2].col = 
		pVtx[3].col = INIT_XCOL_WHITE;

		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

		pVtx += 4;
	}

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();

	//// 項目選択の生成
	//m_pItemSelect = std::make_unique<CItemSelect>();

	//// 項目選択の初期化処理
	//m_pItemSelect->Init();

	// カメラの取得
	CCamera* pCamera = CManager::GetCamera();

	// カメラの初期位置を設定しておく
	pCamera->SetCamParameter(
		TitleCamera::PARAM.eye,
		TitleCamera::PARAM.at,
		TitleCamera::PARAM.rot,
		TitleCamera::PARAM.distance);

	//// 音の取得
	//CSound* pSound = CManager::GetSound();

	//// タイトルBGMの再生
	//if (pSound)
	//{
	//	pSound->Play(CSound::SOUND_LABEL_TITLEBGM);
	//}

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CTitle::Uninit(void)
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

	// 頂点バッファの破棄
	if (m_pVtxBuff != nullptr)
	{
		m_pVtxBuff->Release();
		m_pVtxBuff = nullptr;
	}
}
//=============================================================================
// 更新処理
//=============================================================================
void CTitle::Update(void)
{
	// ロゴの更新処理
	UpdateLogoVertex();

	//// 項目選択の更新処理
	//m_pItemSelect->Update();

	CFade* pFade = CManager::GetFade();
	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

#ifdef _DEBUG
	// 任意のボタンを押したとき
	if (pFade->GetFade() == CFade::FADE_NONE &&
		(pKeyboard->GetTrigger(DIK_RETURN) || pJoypad->GetTrigger(CInputJoypad::JOYKEY_A)))
	{
		// ゲーム画面に移行
		pFade->SetFade(MODE_GAME);
	}
#endif

	// ブロックマネージャーの更新処理
	m_pBlockManager->Update();
}
//=============================================================================
// ロゴの更新処理
//=============================================================================
void CTitle::UpdateLogoVertex(void)
{
	// バックバッファサイズの取得
	float screenW = (float)CManager::GetRenderer()->GetBackBufferWidth();
	float screenH = (float)CManager::GetRenderer()->GetBackBufferHeight();

	float logoW = screenW * TitleUI::LOGO_WIDTH_RATE * CMathConstant::HALF;
	float logoH = screenH * TitleUI::LOGO_HEIGHT_RATE * CMathConstant::HALF;

	float cx = screenW * TitleUI::ANCHOR_X;
	float cy = screenH * TitleUI::ANCHOR_Y;

	VERTEX_2D* pVtx;// 頂点情報へのポインタ

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	pVtx[0].pos = { cx - logoW, cy - logoH, 0.0f };
	pVtx[1].pos = { cx + logoW, cy - logoH, 0.0f };
	pVtx[2].pos = { cx - logoW, cy + logoH, 0.0f };
	pVtx[3].pos = { cx + logoW, cy + logoH, 0.0f };

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();
}
//=============================================================================
// 描画処理
//=============================================================================
void CTitle::Draw(void)
{
	// カメラの取得
	CCamera* pCamera = CManager::GetCamera();

	if (!pCamera->GetMode() == CCamera::MODE_EDIT)
	{
		// カメラの設定
		pCamera->SetCamParameter(
			TitleCamera::PARAM.eye,
			TitleCamera::PARAM.at,
			TitleCamera::PARAM.rot,
			TitleCamera::PARAM.distance);
	}

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 頂点バッファをデータストリームに設定
	pDevice->SetStreamSource(0, m_pVtxBuff, 0, sizeof(VERTEX_2D));

	// 頂点フォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_2D);

	// テクスチャインデックス配列
	int textures[1] = { m_nIdxTextureTitle };

	// 各テクスチャごとに描画
	for (int nCnt = 0; nCnt < TYPE_MAX; nCnt++)
	{
		// テクスチャの設定
		pDevice->SetTexture(0, CManager::GetTexture()->GetAddress(textures[nCnt]));

		// ポリゴンの描画
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, m_vertexRanges[nCnt].start, 2);
	}
}
//=============================================================================
// ライト設定処理
//=============================================================================
void CTitle::ResetLight(void)
{
	// ライトを削除しておく
	CLight::Uninit();

	// ライトの設定
	for (const auto& light : TitleLight::LIGHTS)
	{
		CLight::AddLight(light.type, light.color, light.dir, light.pos);
	}
}
//=============================================================================
// デバイスリセット通知
//=============================================================================
void CTitle::OnDeviceReset(void)
{
	// ライトの再設定処理
	ResetLight();
}
//=============================================================================
// サムネイルリリース通知
//=============================================================================
void CTitle::ReleaseThumbnail(void)
{
	m_pBlockManager->ReleaseThumbnailRenderTarget();
}
//=============================================================================
// サムネイルリセット通知
//=============================================================================
void CTitle::ResetThumbnail(void)
{
	m_pBlockManager->InitThumbnailRenderTarget(CManager::GetRenderer()->GetDevice());
	m_pBlockManager->GenerateThumbnailsForResources(); // キャッシュも再作成
}
