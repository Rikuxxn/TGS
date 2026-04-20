//=============================================================================
//
// プレイヤースペック処理 [SpecPlayer.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "SpecPlayer.h"
#include "Player.h"
#include "MathConst.h"


//=============================================================================
// プレイヤーのHPが少ない(半分以下)
//=============================================================================
bool IsHpFew::IsSatisfiedBy(const CPlayer& player) const
{
	// HPの最大量に設定
	const float nMaxLife = player.GetMaxHp();

	// 現在のHPの取得
	float currentLife = player.GetHp();

	// 半分以下だったら
	return currentLife <= nMaxLife * CMathConstant::HALF;
}
//=============================================================================
// プレイヤーのHPがとても少ない
//=============================================================================
bool IsHpVeryFew::IsSatisfiedBy(const CPlayer& player) const
{
	// HPの最大量に設定
	const float nMaxLife = player.GetMaxHp();

	// 現在のHPの取得
	float currentLife = player.GetHp();

	return currentLife <= VERYFEW_HP;
}
//=============================================================================
// プレイヤーが疲労状態
//=============================================================================
bool IsFatigueSpec::IsSatisfiedBy(const CPlayer& player) const
{
	//return player.IsFatigue();
	return false;
}
//=============================================================================
// プレイヤーが移動中
//=============================================================================
bool IsMovingSpec::IsSatisfiedBy(const CPlayer& player) const
{
	return player.GetIsMoving();
}
