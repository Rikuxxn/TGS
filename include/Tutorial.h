//=============================================================================
//
// チュートリアル処理 [Tutorial.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _TUTORIAL_H_// このマクロ定義がされていなかったら
#define _TUTORIAL_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Scene.h"
#include "BlockManager.h"
#include "Block.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CPlayer;
class CLight;
class CTime;

//*****************************************************************************
// チュートリアルクラス
//*****************************************************************************
class CTutorial : public CScene
{
public:
	CTutorial();
	~CTutorial();

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	virtual void ResetLight(void);
	void Draw(void);
	void OnDeviceReset(void) override;
	void ReleaseThumbnail(void) override;
	void ResetThumbnail(void) override;

	static CTime* GetTime(void) { return m_pTime; }
	CBlockManager* GetBlockManager(void) { return m_pBlockManager; }

private:
	CPlayer*				m_pPlayer;					// プレイヤーへのポインタ
	static CTime*			m_pTime;					// タイムへのポインタ
	CBlockManager*			m_pBlockManager;			// ブロックマネージャーへのポインタ
	CLight*					m_pLight;					// ライトへのポインタ
};

#endif

