//=============================================================================
//
// ステンシルシャドウ処理 [StencilShadow.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _STENCILSHADOW_H_// このマクロ定義がされていなかったら
#define _STENCILSHADOW_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "ObjectX.h"

//*****************************************************************************
// ステンシルシャドウクラス
//*****************************************************************************
class CStencilShadow : public CObjectX
{
public:
	CStencilShadow(int nPriority = PRIORITY::SHADOW);
	~CStencilShadow();

	static CStencilShadow* Create(const char* pFilepath, D3DXVECTOR3 size);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void SetPosition(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetStencilRef(DWORD value) { m_stencilRef = value; }
private:
	static constexpr float	FHD_WIDTH	= 1920.0f;	// FHD画面幅
	static constexpr float	FHD_HEIGHT	= 1080.0f;	// FHD画面高さ

	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;				// 頂点バッファへのポインタ
	D3DXVECTOR3				m_pos;					// 2Dポリゴン描画用位置
	D3DCOLOR				m_col;					// 2Dポリゴン描画用色
	DWORD					m_stencilRef;			// ステンシルバッファの値
	float					m_fWidth;				// 幅(画面サイズの2Dポリゴン用)
	float					m_fHeight;				// 高さ(画面サイズの2Dポリゴン用)

};
#endif
