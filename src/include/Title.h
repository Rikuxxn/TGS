//=============================================================================
//
// タイトル処理 [Title.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _TITLE_H_// このマクロ定義がされていなかったら
#define _TITLE_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Scene.h"
#include "BlockManager.h"
#include "Light.h"
//#include "ItemSelect.h"

//*****************************************************************************
// タイトルクラス
//*****************************************************************************
class CTitle : public CScene
{
public:
	//タイトルの種類
	typedef enum
	{
		TYPE_FIRST = 0,	// タイトル
		TYPE_MAX
	}TYPE;

	CTitle();
	~CTitle();

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void UpdateLogoVertex(void);
	void Draw(void);
	virtual void ResetLight(void);
	void OnDeviceReset(void) override;
	void ReleaseThumbnail(void) override;
	void ResetThumbnail(void) override;

private:

	// 各画像の構造体
	struct ImageInfo
	{
		D3DXVECTOR3 pos;
		float width;
		float height;
	};

	// 頂点の範囲を定義する構造体
	typedef struct
	{
		int start;
		int end;
	}VertexRange;

private:
	VertexRange					 m_vertexRanges[TYPE_MAX];	// タイプごとに頂点範囲を設定
	LPDIRECT3DVERTEXBUFFER9		 m_pVtxBuff;				// 頂点バッファへのポインタ
	int							 m_nIdxTextureTitle;		// テクスチャインデックス
	CBlockManager*				 m_pBlockManager;			// ブロックマネージャーへのポインタ
	CLight*						 m_pLight;					// ライトへのポインタ
	//std::unique_ptr<CItemSelect> m_pItemSelect;				// 項目選択へのポインタ

};

#endif
