//=============================================================================
//
// ゲーム処理 [Game.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _GAME_H_// このマクロ定義がされていなかったら
#define _GAME_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Scene.h"
#include "BlockManager.h"
#include "PauseManager.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CPlayer;
class CTime;
class CLight;

//*****************************************************************************
// ゲームクラス
//*****************************************************************************
class CGame : public CScene
{
public:
	CGame();
	~CGame();

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	virtual void UpdateLight(void);
	void Draw(void);
	void OnDeviceReset(void) override;
	void ReleaseThumbnail(void) override;
	void ResetThumbnail(void) override;

	static CTime* GetTime(void) { return m_pTime; }
	static CBlockManager* GetBlockManager(void) { return m_pBlockManager; }
	static CPauseManager* GetPauseManager(void) { return m_pPauseManager; }
	static bool GetisPaused(void) { return m_isPaused; };
	static void SetEnablePause(bool bPause);

private:
	CPlayer*						 m_pPlayer;				// プレイヤーへのポインタ
	static CTime*					 m_pTime;				// タイムへのポインタ
	static CBlockManager*			 m_pBlockManager;		// ブロックマネージャーへのポインタ
	static CPauseManager*			 m_pPauseManager;		// ポーズマネージャーへのポインタ
	CLight*							 m_pLight;				// ライトへのポインタ
	static bool						 m_isPaused;			// ポーズ中フラグ
};

#endif
