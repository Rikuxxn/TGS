//=============================================================================
//
// プレイヤーの状態処理 [PlayerState.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _PLAYERSTATE_H_// このマクロ定義がされていなかったら
#define _PLAYERSTATE_H_//2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "State.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CPlayer;

//*****************************************************************************
// プレイヤーの待機状態
//*****************************************************************************
class CPlayerStandState :public StateBase<CPlayer>
{
public:

	void OnStart(CPlayer* pPlayer) override;
	void OnUpdate(CPlayer* pPlayer) override;
	void OnExit(CPlayer* /*pPlayer*/) override {}

private:
	static constexpr float	TRIGGER_DISTANCE	= 40.0f;// 判定距離
	static constexpr int	MOTION_CHANGE_FRAME = 10;	// モーション移行フレーム

};

//*****************************************************************************
// プレイヤーの移動状態
//*****************************************************************************
class CPlayerMoveState :public StateBase<CPlayer>
{
public:

	void OnStart(CPlayer* pPlayer) override;
	void OnUpdate(CPlayer* pPlayer) override;
	void OnExit(CPlayer* pPlayer) override;

private:
	static constexpr int	MOTION_CHANGE_FRAME		= 10;		// モーション移行フレーム
};

#endif