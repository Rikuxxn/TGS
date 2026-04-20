//=============================================================================
//
// スカイキューブ処理 [SkyCube.h]
// Author: RIKU TANEKAWA
//
//=============================================================================
#ifndef _SKYCUBE_H_
#define _SKYCUBE_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Object.h"


//*****************************************************************************
// スカイキューブクラス
//*****************************************************************************
class CSkyCube : public CObject
{
public:
	CSkyCube(int nPriority = PRIORITY::BACK);
	~CSkyCube();

	static CSkyCube* Create(void);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

private:
	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;			// 頂点バッファへのポインタ
	int						m_nIdxTexture;		// テクスチャのインデックス
};

#endif

