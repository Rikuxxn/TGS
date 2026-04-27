//=============================================================================
//
// プレイヤースペック処理 [SpecPlayer.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _SPECPLAYER_H_// このマクロ定義がされていなかったら
#define _SPECPLAYER_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Motion.h"
#include "SpecBase.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CPlayer;

//*****************************************************************************
// プレイヤーのHPが少ない(半分以下)
//*****************************************************************************
class IsHpFew : public Specification <CPlayer>
{
public:
	IsHpFew() {}
	~IsHpFew() {}

	bool IsSatisfiedBy(const CPlayer& player) const override;

private:
};

//*****************************************************************************
// プレイヤーのHPがとても少ない
//*****************************************************************************
class IsHpVeryFew : public Specification <CPlayer>
{
public:
	IsHpVeryFew() {}
	~IsHpVeryFew() {}

	bool IsSatisfiedBy(const CPlayer& player) const override;

private:
	static constexpr float VERYFEW_HP = 2.5f;
};

//*****************************************************************************
// プレイヤーが疲労状態
//*****************************************************************************
class IsFatigueSpec : public Specification<CPlayer>
{
public:
	IsFatigueSpec() {}
	~IsFatigueSpec() {}

	bool IsSatisfiedBy(const CPlayer& player) const override;
};

//*****************************************************************************
// プレイヤーが移動中
//*****************************************************************************
class IsMovingSpec : public Specification<CPlayer>
{
public:
	IsMovingSpec() {}
	~IsMovingSpec() {}

	bool IsSatisfiedBy(const CPlayer& player) const override;

};

#endif