//=============================================================================
//
// リザルト処理 [Result.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _RESULT_H_// このマクロ定義がされていなかったら
#define _RESULT_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Scene.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CLight;
class CBlockManager;


//*****************************************************************************
// リザルトクラス
//*****************************************************************************
class CResult : public CScene
{
public:
	CResult();
	~CResult();

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);


	static void ResetLight(void);
	void OnDeviceReset(void) override;
	void ReleaseThumbnail(void) override;
	void ResetThumbnail(void) override;

private:
	CLight*				m_pLight;			// ライトへのポインタ
	CBlockManager*		m_pBlockManager;	// ブロックマネージャーへのポインタ
};

#endif
