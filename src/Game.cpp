//=============================================================================
//
// ゲーム処理 [Game.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Game.h"
#include "Manager.h"
#include "Result.h"
#include "CharacterManager.h"
#include "Player.h"
#include "Ui.h"
#include "BlockList.h"
#include "SkyCube.h"
#include "Timer.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
CTime* CGame::m_pTime = nullptr;					// タイムへのポインタ
CBlockManager* CGame::m_pBlockManager= nullptr;		// ブロックマネージャーへのポインタ
CPauseManager* CGame::m_pPauseManager = nullptr;	// ポーズマネージャーへのポインタ
bool CGame::m_isPaused = false;						// trueならポーズ中

//*****************************************************************************
// 名前空間
//*****************************************************************************
namespace GameWorld
{
	const D3DXVECTOR3	DOME_POS{ D3DXVECTOR3(0.0f, 0.0f, 0.0f) };
	const D3DXVECTOR3	PLAYER_POS{ 0.0f, 30.0f, -320.0f };
	const D3DXVECTOR3	PLAYER_ROT{ 0.0f, 180.0f, 0.0f };
	const D3DXVECTOR3	ENEMY_LEADER_POS{ 0.0f, 15.0f, 300.0f };
	const D3DXVECTOR3	ENEMY_ROT{ 0.0f, 0.0f, 0.0f };
	const D3DXCOLOR		PARTICLE_COLOR{ 0.8f, 0.8f, 0.8f, 0.8f };
	constexpr int		DOME_RADIUS		= 1000;
	constexpr int		TIME_MINUTES	= 3;
	constexpr int		TIME_SECONDS	= 0;
	constexpr int		PARTICLE_NUM	= 1;
	constexpr float		BASE_X			= 760.0f;
	constexpr float		BASE_Y			= 10.0f;
	constexpr float		DIGIT_WIDTH		= 42.0f;
	constexpr float		DIGIT_HEIGHT	= 58.0f;
}

namespace GameUI
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
	const D3DXCOLOR COLOR = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.0f);

	// スライドスピード
	constexpr float RULE_DURATION		= 60.0f;
	constexpr float COMPLETE_DURATION	= 60.0f;
	constexpr float FAILURE_DURATION	= 120.0f;

	// 表示時間
	constexpr int START_TIME			= 180;
	constexpr int FAILURE_DELAY_TIME	= 120;
	constexpr int FAILURE_TIME			= 180;
	constexpr int COMPLETE_TIME			= 240;
}

namespace LightParam
{
	// 時間境界
	constexpr float DAY_HALF_RATE = 0.5f;

	// メインライト色
	const D3DXCOLOR EVENING_COLOR(1.0f, 0.65f, 0.35f, 1.0f);
	const D3DXCOLOR NIGHT_COLOR(0.15f, 0.18f, 0.35f, 1.0f);
	const D3DXCOLOR MORNING_COLOR(0.95f, 0.8f, 0.7f, 1.0f);

	// メインライト方向
	const D3DXVECTOR3 DIR_EVENING(0.5f, -1.0f, 0.3f);
	const D3DXVECTOR3 DIR_NIGHT(0.0f, -1.0f, 0.0f);
	const D3DXVECTOR3 DIR_MORNING(-0.3f, -1.0f, -0.2f);

	// サブライト（空）色
	const D3DXCOLOR SKY_EVENING(0.4f, 0.45f, 0.8f, 1.0f);
	const D3DXCOLOR SKY_NIGHT(0.1f, 0.15f, 0.3f, 1.0f);
	const D3DXCOLOR SKY_MORNING(0.6f, 0.7f, 1.0f, 1.0f);

	// 補助光
	const D3DXVECTOR3 SUB_DIR(-0.3f, 0.0f, -0.7f);

	constexpr float SUB_BASE_R = 0.5f;
	constexpr float SUB_WARM_R = 0.2f;
	constexpr float SUB_G = 0.3f;
	constexpr float SUB_B = 0.25f;
}

//=============================================================================
// コンストラクタ
//=============================================================================
CGame::CGame() : CScene(CScene::MODE_GAME)
{
	// 値のクリア
	m_pLight			= nullptr;					// ライトへのポインタ
}
//=============================================================================
// デストラクタ
//=============================================================================
CGame::~CGame()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CGame::Init(void)
{
	// 名前空間ゲームワールドの使用
	using namespace GameWorld;

	// 名前空間ゲームUIの使用
	using namespace GameUI;

	// ブロックマネージャーの生成
	m_pBlockManager = new CBlockManager;

	// ブロックマネージャーの初期化
	m_pBlockManager->Init();

	// ライトの生成
	m_pLight = new CLight;

	// ライトの初期化
	m_pLight->Init();

	// スカイキューブの生成
	CSkyCube::Create();

	//// 壁などの配置情報の読み込み
	//m_pBlockManager->LoadFromJson("data/StageInfo/Game_BlockInfo.json");

	// キャラクターマネージャーの生成
	auto& charaMgr = CCharacterManager::GetInstance();

	// プレイヤーの生成
	m_pPlayer = CPlayer::Create(PLAYER_POS, PLAYER_ROT);
	charaMgr.AddCharacter(m_pPlayer);

	m_pPlayer->SetControlFlag(true);

	// タイムの生成
	m_pTime = CTime::Create(TIME_MINUTES, TIME_SECONDS, BASE_X, BASE_Y, DIGIT_WIDTH, DIGIT_HEIGHT, false);


	//// UI生成
	//auto testUI = CUITexture::Create("data/TEXTURE/test.png", 
	//	Pos::RULE_ANCHOR_X, Pos::RULE_ANCHOR_Y, COLOR, Size::RULE_W, Size::RULE_H);

	//// UI登録
	//CUIManager::GetInstance()->AddUI("TEST", testUI);

	//// 生成直後に各UIの設定をする
	//testUI->Hide();// 最初は非表示スタート


	// ポーズマネージャーの生成
	m_pPauseManager = new CPauseManager();

	// ポーズマネージャーの初期化
	m_pPauseManager->Init();

	//// 音の取得
	//CSound* pSound = CManager::GetSound();

	//// ゲームBGMの再生
	//if (pSound)
	//{
	//	pSound->Play(CSound::SOUND_LABEL_GAMEBGM);
	//}

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CGame::Uninit(void)
{
	// キャラクターマネージャーの破棄
	CCharacterManager::GetInstance().Destroy();

	// ジョイパッドの取得
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

	// 振動停止
	pJoypad->StopVibration();

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

	// ポーズマネージャーの破棄
	if (m_pPauseManager != nullptr)
	{
		// ポーズマネージャーの終了処理
		m_pPauseManager->Uninit();

		delete m_pPauseManager;
		m_pPauseManager = nullptr;
	}
}
//=============================================================================
// 更新処理
//=============================================================================
void CGame::Update(void)
{
	// 名前空間ゲームワールドの使用
	using namespace GameWorld;

	CFade* pFade = CManager::GetFade();
	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

	// ライトの更新
	UpdateLight();

	// TABキーでポーズON/OFF
	if (pKeyboard->GetTrigger(DIK_TAB) || pJoypad->GetTrigger(CInputJoypad::JOYKEY_START))
	{
		// ポーズ切り替え前の状態を記録
		bool wasPaused = m_isPaused;

		m_isPaused = !m_isPaused;

		// ポーズ状態に応じて音を制御
		if (m_isPaused && !wasPaused)
		{
			// 振動停止
			pJoypad->StopVibration();

			// 一時停止する
			CManager::GetSound()->PauseAll();
		}
		else if (!m_isPaused && wasPaused)
		{
			// 再開する
			CManager::GetSound()->ResumeAll();
		}
	}

	// ブロックマネージャーの更新処理
	m_pBlockManager->Update();


	//// UIの取得
	//auto escape_xinput = CUIManager::GetInstance()->GetUI("Escape_XInput");
	//auto escape_keyboard = CUIManager::GetInstance()->GetUI("Escape_Keyboard");


	if (pFade->GetFade() == CFade::FADE_NONE && pKeyboard->GetTrigger(DIK_RETURN))
	{
		// リザルト画面に移行
		pFade->SetFade(MODE_RESULT);
	}
}
//=============================================================================
// ライト更新処理
//=============================================================================
void CGame::UpdateLight(void)
{
	float progress = m_pTime->GetProgress(); // 0.0～0.1

	// 各時間帯のメインライト色
	D3DXCOLOR mainColor;

	// 時間帯ごとに補間
	if (progress < CTime::NIGHT_START_RATE)
	{// 夕方
		float t = progress / CTime::NIGHT_START_RATE;
		D3DXColorLerp(&mainColor, &LightParam::EVENING_COLOR, &LightParam::NIGHT_COLOR, t);
	}
	else if (progress < CTime::NIGHT_END_RATE)
	{// 夜
		float t = (progress - CTime::NIGHT_START_RATE) / (CTime::NIGHT_END_RATE - CTime::NIGHT_START_RATE);
		D3DXColorLerp(&mainColor, &LightParam::NIGHT_COLOR, &LightParam::MORNING_COLOR, t);
	}
	else
	{// 明け方
		float t = (progress - CTime::NIGHT_END_RATE) / (1.0f - CTime::NIGHT_END_RATE);
		D3DXColorLerp(&mainColor, &LightParam::MORNING_COLOR, &LightParam::EVENING_COLOR, t);
	}

	// 光の向き補間
	D3DXVECTOR3 mainDir;

	if (progress < LightParam::DAY_HALF_RATE)
	{
		float t = progress / LightParam::DAY_HALF_RATE;
		D3DXVec3Lerp(&mainDir, &LightParam::DIR_EVENING, &LightParam::DIR_NIGHT, t);
	}
	else
	{
		float t = (progress - LightParam::DAY_HALF_RATE) / LightParam::DAY_HALF_RATE;
		D3DXVec3Lerp(&mainDir, &LightParam::DIR_NIGHT, &LightParam::DIR_MORNING, t);
	}

	// 正規化
	D3DXVec3Normalize(&mainDir, &mainDir);

	// 再設定
	CLight::Uninit();

	m_pBlockManager->UpdateLight();

	// メインライト
	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		mainColor,
		mainDir,
		INIT_VEC3
	);

	// サブライト
	D3DXCOLOR skyColor;

	if (progress < LightParam::DAY_HALF_RATE)
	{
		D3DXColorLerp(&skyColor, &LightParam::SKY_EVENING, &LightParam::SKY_NIGHT, 
			progress / LightParam::DAY_HALF_RATE);
	}
	else
	{
		D3DXColorLerp(&skyColor, &LightParam::SKY_NIGHT, &LightParam::SKY_MORNING, 
			(progress - LightParam::DAY_HALF_RATE) / LightParam::DAY_HALF_RATE);
	}

	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		skyColor,
		D3DXVECTOR3(0.0f, -1.0f, 0.0f),
		INIT_VEC3
	);

	// 補助光
	float warmFactor = 1.0f - fabs(progress - LightParam::DAY_HALF_RATE) * 2.0f;
	warmFactor = std::max(0.0f, warmFactor);

	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		D3DXCOLOR(
			LightParam::SUB_BASE_R + LightParam::SUB_WARM_R * warmFactor,
			LightParam::SUB_G,
			LightParam::SUB_B,
			1.0f),
		LightParam::SUB_DIR,
		INIT_VEC3
	);
}
//=============================================================================
// 描画処理
//=============================================================================
void CGame::Draw(void)
{
	// ポーズ中だったら
	if (m_isPaused)
	{
		// ポーズマネージャーの描画処理
		m_pPauseManager->Draw();
	}
}
//=============================================================================
// デバイスリセット通知
//=============================================================================
void CGame::OnDeviceReset(void)
{
	// ゲームライトの更新処理
	UpdateLight();
}
//=============================================================================
// ポーズの設定
//=============================================================================
void CGame::SetEnablePause(bool bPause)
{
	m_isPaused = bPause;

	if (bPause)
	{
		// 音を一時停止
		CManager::GetSound()->PauseAll();
	}
	else
	{
		// 音を再開
		CManager::GetSound()->ResumeAll();
	}
}
//=============================================================================
// サムネイルリリース通知
//=============================================================================
void CGame::ReleaseThumbnail(void)
{
	m_pBlockManager->ReleaseThumbnailRenderTarget();
}
//=============================================================================
// サムネイルリセット通知
//=============================================================================
void CGame::ResetThumbnail(void)
{
	m_pBlockManager->InitThumbnailRenderTarget(CManager::GetRenderer()->GetDevice());
	m_pBlockManager->GenerateThumbnailsForResources(); // キャッシュも再作成
}
