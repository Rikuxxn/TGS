//=============================================================================
//
// ブロック処理 [Block.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _BLOCK_H_
#define _BLOCK_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "ObjectX.h"
#include "DebugProc3D.h"
#include "unordered_map"
#include "functional"
#include "json.hpp"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CBlock;

//*****************************************************************************
// 型定義
//*****************************************************************************
using BlockCreateFunc = std::function<CBlock* ()>;
using json = nlohmann::json;


//*****************************************************************************
// ブロッククラス
//*****************************************************************************
class CBlock : public CObjectX
{
public:
	CBlock(int nPriority = PRIORITY::OBJECTS);
	virtual ~CBlock() = default;

	//*****************************************************************************
	// ブロックの種類
	//*****************************************************************************
	typedef enum
	{
		TYPE_WALL_01 = 0,
		TYPE_MAX
	}TYPE;

	// OBB情報構造体
	struct OBB
	{
		D3DXVECTOR3 center;
		D3DXVECTOR3 axis[3];  // x,y,z軸の方向ベクトル
		D3DXVECTOR3 halfSize;
	};

	static CBlock* Create(const char* pFilepath, D3DXVECTOR3 pos, D3DXVECTOR3 rot,D3DXVECTOR3 size, TYPE type);	// ブロックの生成
	virtual btCollisionShape* CreateCollisionShape(const D3DXVECTOR3& size)
	{ 
		return new btBoxShape(btVector3(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f));	// デフォルトはボックス 派生クラスで同じのを作ってShapeを設定
	}
	void CreatePhysics(const D3DXVECTOR3& pos, const D3DXVECTOR3& size);				// コライダーの生成
	void CreatePhysicsFromScale(const D3DXVECTOR3& scale);								// ブロックスケールによるコライダーの生成
	virtual void OnPhysicsCreated(void) {}												// ブロックの特殊処理
	virtual void OnPhysicsReleased(void) {}												// ブロックの特殊処理の破棄
	static void InitFactory(void);
	HRESULT Init(void);
	void Kill(void) { m_isDead = true; }												// ブロック削除
	void Uninit(void);
	void Update(void);
	void UpdateTransform(void);
	void UpdateCollider(void);
	void Draw(void);
	void DrawCollider(void);
	void ReleasePhysics(void);															// Physics破棄用
	virtual void Respawn(D3DXVECTOR3 resPos);
	virtual void DrawCustomUI(void) {}													// 派生クラスのGUI特殊処理用
	bool IsHitOBBvsAABB(const OBB& obb, const D3DXVECTOR3& aabbMin, const D3DXVECTOR3& aabbMax);
	virtual void UpdateLight(void) {}
	virtual void SaveToJson(json& b);
	virtual void LoadFromJson(const json& b);

	//*****************************************************************************
	// flagment関数
	//*****************************************************************************
	bool IsSelected(void) const { return m_bSelected; }										// ブロックが選択中のフラグを返す
	bool IsEditMode(void) const { return m_isEditMode; }									// エディット中かどうか
	virtual bool IsDynamicBlock(void) const { return false; }								// 動的ブロックの判別
	virtual bool IsEnd(void) { return false; }
	virtual bool IsGet(void) { return false; }
	virtual bool IsSwitchOn(void) { return false; }

	//*****************************************************************************
	// setter関数
	//*****************************************************************************
	void SetType(TYPE type) { m_Type = type; }												// タイプの設定
	void SetSelected(bool flag) { m_bSelected = flag; }										// 選択中のフラグを返す
	void SetColliderSize(const D3DXVECTOR3& size) { m_colliderSize = size; }				// コライダーサイズの設定
	void SetColliderManual(const D3DXVECTOR3& newSize);										// コライダーサイズの手動設定用
	void SetColliderOffset(const D3DXVECTOR3& offset) { m_colliderOffset = offset; }		// コライダーのオフセットの設定
	void SetEditMode(bool enable);

	//*****************************************************************************
	// getter関数
	//*****************************************************************************
	virtual D3DXCOLOR GetCol(void) const override;											// カラーの取得
	TYPE GetType(void) const { return m_Type; }												// タイプの取得
	btRigidBody* GetRigidBody(void) const { return m_pRigidBody; }							// RigidBodyの取得
	D3DXVECTOR3 GetColliderSize(void) const { return m_colliderSize; }						// コライダーサイズの取得
	D3DXVECTOR3 GetColliderOffset(void) const { return m_colliderOffset; }					// コライダーのオフセットの取得
	virtual btScalar GetMass(void) const { return DEFAULT_MASS; }							// 質量の取得
	bool IsDead(void) const { return m_isDead; }											// 削除予約の取得
	D3DXMATRIX GetWorldMatrix(void);														// マトリックス取得

	virtual int GetCollisionFlags(void) const { return 0; }									// デフォルトはフラグなし
	virtual btVector3 GetLinearFactor(void) const { return btVector3(1.0f, 1.0f, 1.0f); }
	virtual btVector3 GetAngularFactor(void) const { return btVector3(1.0f, 1.0f, 1.0f); }
	virtual btScalar GetRollingFriction(void) const { return DEFAULT_ROLLING_FRICTION; }
	virtual btScalar GetFriction(void) const { return DEFAULT_FRICTION; }

private:
	static constexpr float	DEFAULT_MASS				= 2.0f;				// デフォルト質量
	static constexpr float	DEFAULT_ROLLING_FRICTION	= 0.7f;				// デフォルト回転摩擦
	static constexpr float	DEFAULT_FRICTION			= 1.0f;				// デフォルト摩擦
	static constexpr float	LINEAR_DAMPING				= 0.1f;				// 線形減衰
	static constexpr float	ANGLAR_DAMPING				= 0.5f;				// 角減衰
	static constexpr int	AXIS						= 3;				// 各軸

	//*****************************************************************************
	// 基本情報
	//*****************************************************************************
	TYPE				m_Type;											// 種類
	D3DXCOLOR			m_col;											// アルファ値
	D3DXCOLOR			m_baseCol;										// ベースのアルファ値
	bool				m_bSelected;									// 選択フラグ
	bool				m_isEditMode;									// 編集中かどうか
	bool				m_isDead;										// 削除予約フラグ

	//*****************************************************************************
	// Physics
	//*****************************************************************************
	btRigidBody*		m_pRigidBody;									// 剛体へのポインタ
	btCollisionShape*	m_pShape;										// 当たり判定の形へのポインタ

	D3DXVECTOR3			m_prevSize;										// 前回のサイズ
	D3DXVECTOR3			m_colliderSize;									// コライダーサイズ
	D3DXVECTOR3			m_colliderOffset;								// コライダーの位置

	//*****************************************************************************
	// Debug
	//*****************************************************************************
	CDebugProc3D*		m_pDebug3D;										// 3Dデバッグ表示へのポインタ

	//*****************************************************************************
	// Factory
	//*****************************************************************************
	static std::unordered_map<TYPE, BlockCreateFunc> m_BlockFactoryMap;	// ブロックファクトリー
};

#endif