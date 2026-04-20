//=============================================================================
//
// プレイヤー処理 [Player.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _PLAYER_H_// このマクロ定義がされていなかったら
#define _PLAYER_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "State.h"
#include "Character.h"
#include "BlockList.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CBlock;
class CDebugProc3D;
class CStencilShadow;
class CMotion;
class CModel;

//*****************************************************************************
// プレイヤークラス
//*****************************************************************************
class CPlayer : public CCharacter
{
public:
	CPlayer();
	~CPlayer();

	static constexpr float PLAYER_SPEED			= 35.0f;		// 通常移動時スピード
	static constexpr float INJURY_SPEED			= 12.0f;		// 負傷時スピード
	static constexpr float STEALTH_SPEED		= 8.0f;			// 忍び移動時スピード
	static constexpr float DECELERATION_RATE	= 0.82f;		// 減速率
	static constexpr float ACCEL_RATE			= 0.15f;		// スピードの補間率
	static constexpr float ROT_SPEED			= 0.2f;			// 向きの更新スピード

	// プレイヤーモーションの種類
	typedef enum
	{
		NEUTRAL = 0,		// 待機
		MOVE,				// 移動
		MAX
	}PLAYER_MOTION;

	// 入力データ構造体
	struct InputData
	{
		D3DXVECTOR3 moveDir;	// 前後移動ベクトル
	};

	static CPlayer* Create(D3DXVECTOR3 pos, D3DXVECTOR3 rot);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void Respawn(D3DXVECTOR3 pos);
	void ApplyDeceleration(void);

	//*****************************************************************************
	// flagment関数
	//*****************************************************************************


	//*****************************************************************************
	// setter関数
	//*****************************************************************************
	void SetControlFlag(bool flag) { m_canControl = flag; }
	void SetOnGround(bool flag) { m_bOnGround = flag; }

	//*****************************************************************************
	// getter関数
	//*****************************************************************************
	CMotion* GetMotion(void) const { return m_pMotion; }
	CModel** GetModels(void) { return m_apModel; }
	InputData GatherInput(void);
	D3DXVECTOR3 GetForward(void);
	int GetNumModels(void) { return m_nNumModel; }
	bool GetControlFlag(void) { return m_canControl; }
	bool GetOnGround(void) { return m_bOnGround; }
	bool GetIsMoving(void) const { return m_bIsMoving; }

	// ステート用のフラグ更新処理
	void UpdateMovementFlags(const D3DXVECTOR3& moveDir);
	//void Damage(float fDamage) override;

private:
	static constexpr int	MAX_PARTS				= 32;		// 最大パーツ数
	static constexpr int	EFFECT_CREATE_NUM		= 3;		// エフェクト
	static constexpr int	HEARTBEART_INTERVAL_1	= 60;		// 心音のインターバル1段階目
	static constexpr int	HEARTBEART_INTERVAL_2	= 30;		// 心音のインターバル2段階目
	static constexpr int	SPAWN_INTERVAL			= 15;		// 生成インターバル
	static constexpr float	CAPSULE_RADIUS			= 10.5f;	// カプセルコライダーの半径
	static constexpr float	CAPSULE_HEIGHT			= 45.5f;	// カプセルコライダーの高さ
	static constexpr float	RESPAWN_HEIGHT			= -280.0f;	// リスポーンする高さ
	static constexpr float	COLLIDER_OFFSET			= 35.0f;	// コライダーオフセット位置
	static constexpr float	HEIGHT_STEP				= 30.0f;	// 高さの増加量
	static constexpr float	OFFSET_POS				= 40.0f;	// 生成オフセット位置
	static constexpr float	MASS					= 2.0f;		// 質量
	static constexpr float	ROT_DEST				= 180.0f;	// 目標の向き

	D3DXMATRIX					m_mtxWorld;						// ワールドマトリックス
	CModel*						m_apModel[MAX_PARTS];			// モデル(パーツ)へのポインタ
	CStencilShadow*				m_pShadowS;						// ステンシルシャドウへのポインタ
	CMotion*					m_pMotion;						// モーションへのポインタ
	CDebugProc3D*				m_pDebug3D;						// 3Dデバッグ表示へのポインタ
	int							m_nNumModel;					// モデル(パーツ)の総数
	bool						m_bIsMoving;					// 移動入力フラグ
	bool						m_bOnGround;					// 接地フラグ
	bool						m_canControl;					// 操作フラグ

	// ステートを管理するクラスのインスタンス
	StateMachine<CPlayer> m_stateMachine;
};

#endif
