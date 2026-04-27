//=============================================================================
//
// ビルボード処理 [ObjectBillboard.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _OBJECTBILLBOARD_H_// このマクロ定義がされていなかったら
#define _OBJECTBILLBOARD_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Object.h"


//*****************************************************************************
// ビルボードクラス
//*****************************************************************************
class CObjectBillboard : public CObject
{
public:
	CObjectBillboard(int nPriority = PRIORITY::MESH);
	~CObjectBillboard();

	static CObjectBillboard* Create(const char* path, D3DXVECTOR3 pos, float fWidth, float fHeight);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void UpdateTurn(void);
	void Draw(void);
	void UpdateGuageVtx(float fRate);
	void UpdateFrame(void);

	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetRot(D3DXVECTOR3 rot) { m_rot = rot; }
	void SetCol(D3DXCOLOR col) { m_col = col; }
	void SetSize(float fRadius) { m_fSize = fRadius; }
	void SetSize(float fWidth, float fHeight) { m_fWidth = fWidth; m_fHeight = fHeight; }
	void SetPath(const char* path);

	D3DXVECTOR3 GetPos(void) { return m_pos; }
	D3DXVECTOR3 GetRot(void) { return m_rot; }
	D3DXCOLOR GetCol(void) { return m_col; }

private:
	static constexpr float OFFSET	= 1.0f; // フレームの線幅分のオフセット


	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;			// 頂点バッファへのポインタ
	D3DXVECTOR3				m_pos;				// 位置
	D3DXVECTOR3				m_rot;				// 向き
	D3DXCOLOR				m_col;				// 色
	D3DXMATRIX				m_mtxWorld;			// ワールドマトリックス
	float					m_fSize;			// サイズ(エフェクト半径)
	float					m_fWidth;			// サイズ
	float					m_fHeight;			// サイズ(ビルボード)
	int						m_nIdxTexture;		// テクスチャインデックス
	char					m_szPath[MAX_PATH];	// ファイルパス
};

#endif