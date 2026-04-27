//=============================================================================
//
// チュートリアル処理 [Tutorial.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Tutorial.h"
#include "Manager.h"
#include "Result.h"
#include "CharacterManager.h"
#include "Player.h"
#include "BlockList.h"
#include "Ui.h"
#include "Parameter.h"
#include "SkyCube.h"
#include "Timer.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
CTime* CTutorial::m_pTime = nullptr;					// タイムへのポインタ


//*****************************************************************************
// 名前空間
//*****************************************************************************
namespace TutorialWorld
{
	const D3DXVECTOR3	PLAYER_POS{ 0.0f, 30.0f, -300.0f };
	const D3DXVECTOR3	PLAYER_ROT{ 0.0f, 180.0f, 0.0f };
	constexpr int		TIME_MINUTES	= 3;
	constexpr int		TIME_SECONDS	= 0;
	constexpr float		BASE_X			= 760.0f;
	constexpr float		BASE_Y			= 10.0f;
	constexpr float		DIGIT_WIDTH		= 42.0f;
	constexpr float		DIGIT_HEIGHT	= 58.0f;
}

namespace TutorialLight
{
	const LightParam LIGHTS[] =
	{
		{ D3DLIGHT_DIRECTIONAL, {1.0f, 0.65f, 0.35f, 1.0f}, {0.5f, -1.0f, 0.3f}, {0,300,0} },
		{ D3DLIGHT_DIRECTIONAL, {0.4f, 0.45f, 0.8f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0,0,0} },
		{ D3DLIGHT_DIRECTIONAL, {0.7f, 0.3f, 0.25f, 1.0f}, {-0.3f, 0.0f, -0.7f}, {0,0,0} }
	};
}

namespace TutorialUI
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
CTutorial::CTutorial() : CScene(CScene::MODE_TUTORIAL)
{
	// 値のクリア
	m_pLight					= nullptr;	// ライトへのポインタ
}
//=============================================================================
// デストラクタ
//=============================================================================
CTutorial::~CTutorial()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CTutorial::Init(void)
{
	// 名前空間チュートリアルUIの使用
	using namespace TutorialUI;

	// ブロックマネージャーの生成
	m_pBlockManager = new CBlockManager;

	// ブロックマネージャーの初期化
	m_pBlockManager->Init();

	// ライトの生成
	m_pLight = new CLight;

	// ライトの初期化
	m_pLight->Init();

	// ライトの設定処理
	ResetLight();

	// スカイキューブの生成
	CSkyCube::Create();

	//// 配置情報の読み込み
	//m_pBlockManager->LoadFromJson("data/StageInfo/Tutorial_BlockInfo.json");

	// キャラクターマネージャーの生成
	auto& charaMgr = CCharacterManager::GetInstance();

	// プレイヤーの生成
	m_pPlayer = CPlayer::Create(TutorialWorld::PLAYER_POS, TutorialWorld::PLAYER_ROT);
	charaMgr.AddCharacter(m_pPlayer);

	// タイムの生成
	m_pTime = CTime::Create(
		TutorialWorld::TIME_MINUTES, 
		TutorialWorld::TIME_SECONDS, 
		TutorialWorld::BASE_X,
		TutorialWorld::BASE_Y, 
		TutorialWorld::DIGIT_WIDTH,
		TutorialWorld::DIGIT_HEIGHT,
		false);

	//// 「UI生成
	//auto testUI = CUITexture::Create("data/TEXTURE/test.png", 
	//	Pos::TUTORIAL_X, Pos::TUTORIAL_Y, TutorialUI::COLOR, Size::TUTORIAL_W, Size::TUTORIAL_H);


	//// UI登録
	//CUIManager::GetInstance()->AddUI("TEST", testUI);

	//// UI初期設定
	//testUI->Hide();

	//// 音の取得
	//CSound* pSound = CManager::GetSound();

	//// チュートリアルBGMの再生
	//if (pSound)
	//{
	//	pSound->Play(CSound::SOUND_LABEL_TUTORIALBGM);
	//}

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CTutorial::Uninit(void)
{
	// キャラクターマネージャーの破棄
	CCharacterManager::GetInstance().Destroy();

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
void CTutorial::Update(void)
{
	// 名前空間TutorialWorldの使用
	using namespace TutorialWorld;

	CFade* pFade = CManager::GetFade();
	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

	//// UIの取得
	//auto testUI = CUIManager::GetInstance()->GetUI("TEST");

	// ブロックマネージャーの更新処理
	m_pBlockManager->Update();

	// 任意のボタンを押したとき
	if (pFade->GetFade() == CFade::FADE_NONE &&
		(pKeyboard->GetTrigger(DIK_TAB) || pJoypad->GetTrigger(CInputJoypad::JOYKEY_START)))
	{
		// ゲーム画面に移行
		pFade->SetFade(MODE_GAME);
	}

#ifdef _DEBUG
	CInputKeyboard* pInputKeyboard = CManager::GetInputKeyboard();

	if (pFade->GetFade() == CFade::FADE_NONE && pInputKeyboard->GetTrigger(DIK_F1))
	{
		// ゲーム画面に移行
		pFade->SetFade(MODE_GAME);
	}
#endif

}
//=============================================================================
// ライトの設定処理
//=============================================================================
void CTutorial::ResetLight(void)
{
	// 再設定
	CLight::Uninit();

	// ライトの設定
	for (const auto& light : TutorialLight::LIGHTS)
	{
		CLight::AddLight(light.type, light.color, light.dir, light.pos);
	}
}
//=============================================================================
// 描画処理
//=============================================================================
void CTutorial::Draw(void)
{

}
//=============================================================================
// デバイスリセット通知
//=============================================================================
void CTutorial::OnDeviceReset(void)
{
	// ライトの設定処理
	ResetLight();
}
//=============================================================================
// サムネイルリリース通知
//=============================================================================
void CTutorial::ReleaseThumbnail(void)
{
	m_pBlockManager->ReleaseThumbnailRenderTarget();
}
//=============================================================================
// サムネイルリセット通知
//=============================================================================
void CTutorial::ResetThumbnail(void)
{
	m_pBlockManager->InitThumbnailRenderTarget(CManager::GetRenderer()->GetDevice());
	m_pBlockManager->GenerateThumbnailsForResources(); // キャッシュも再作成
}

