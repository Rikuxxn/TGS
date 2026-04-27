//=============================================================================
//
// 計算用定数処理 [MathConst.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _MATHCONST_H_// このマクロ定義がされていなかったら
#define _MATHCONST_H_// 2重インクルード防止のマクロ定義


//*****************************************************************************
// 計算用定数クラス
//*****************************************************************************
class CMathConstant
{
public:
	// int型の定数
	static constexpr int I_ANGLE_HALF	= 180;		// int型の角度の半分値
	static constexpr int I_ANGLE_MAX	= 360;		// int型の角度の最大値
	static constexpr int I_DOUBLE		= 2;		// int型の二倍値
	static constexpr int I_RAND_NUM		= 10000;	// 0.0～1.0の乱数用

	// float型の定数
	static constexpr float F_ANGLE_HALF = 180.0f;	// float型の角度の半分値
	static constexpr float F_ANGLE_MAX	= 360.0f;	// float型の角度の最大値
	static constexpr float F_DOUBLE		= 2.0f;		// float型の二倍値
	static constexpr float HALF			= 0.5f;		// 半分値
	static constexpr float F_RAND_NUM	= 10000.0f;	// 0.0～1.0の乱数用
	static constexpr float PI_HALF		= 1.57f;	// 円周率の半分
};

#endif