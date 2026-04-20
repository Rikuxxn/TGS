//=============================================================================
//
// プレイヤーの状態処理 [PlayerState.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "PlayerState.h"
#include "BlockList.h"
#include "SpecBase.h"
#include "SpecPlayer.h"
#include "CharacterManager.h"
#include "Sound.h"
#include "Input.h"
#include "Manager.h"
#include "Player.h"

//=============================================================================
// 待機状態の開始処理
//=============================================================================
void CPlayerStandState::OnStart(CPlayer* pPlayer)
{
	// 待機モーション
	pPlayer->GetMotion()->StartBlendMotion(CPlayer::NEUTRAL, MOTION_CHANGE_FRAME);
}
//=============================================================================
// 待機状態の更新処理
//=============================================================================
void CPlayerStandState::OnUpdate(CPlayer* pPlayer)
{
	// 入力を取得
	CPlayer::InputData input = pPlayer->GatherInput();

	// 入力状態を更新
	pPlayer->UpdateMovementFlags(input.moveDir);

	// プレイヤーHPが少ない
	IsHpFew hpFew;

	// 減速処理
	pPlayer->ApplyDeceleration();

	// 移動入力があったら移動ステートに移行
	if (pPlayer->GetIsMoving())
	{
		if (hpFew.IsSatisfiedBy(*pPlayer))
		{
			//// 疲労状態へ移行
			//m_pMachine->ChangeState<CPlayerFatigueState>();
		}
		else
		{
			// 移動状態へ移行
			m_pMachine->ChangeState<CPlayerMoveState>();
		}

		return;
	}
}


//=============================================================================
// 移動状態の開始処理
//=============================================================================
void CPlayerMoveState::OnStart(CPlayer* pPlayer)
{
	// 移動モーション
	pPlayer->GetMotion()->StartBlendMotion(CPlayer::MOVE, MOTION_CHANGE_FRAME);
}
//=============================================================================
// 移動状態の更新処理
//=============================================================================
void CPlayerMoveState::OnUpdate(CPlayer* pPlayer)
{
	// 入力取得
	CPlayer::InputData input = pPlayer->GatherInput();

	// フラグ更新
	pPlayer->UpdateMovementFlags(input.moveDir);

	// プレイヤーHPが少ない
	IsHpFew hpFew;

	// 満たしていたら
	if (hpFew.IsSatisfiedBy(*pPlayer))
	{
		//// 疲労状態
		//m_pMachine->ChangeState<CPlayerFatigueState>();
		return;
	}

	//// 疲労に応じたスピード
	//float speedRate = 1.0f - treasureCount * DEC_SPEED_RATE;// 5%ずつ低下
	//speedRate = std::max(speedRate, MAX_DEC_RATE); // 最大50%

	//// スピードレートの設定
	//speedRate = MOVE_SPEED_RATE;

	//// 埋蔵金を取るたびにレートを落とす
	//float NewSpeedRate = speedRate - treasureCount * DEC_NEWSPEED_RATE;// 8%ずつ低下
	//NewSpeedRate = std::max(NewSpeedRate, MAX_DEC_RATE); // 最大50%

	//speedRate = NewSpeedRate;

	//// モーションスピードを遅くしていく
	//pPlayer->GetMotion()->SetMotionSpeedRate(speedRate);

	// 目標速度計算
	float moveSpeed = CPlayer::PLAYER_SPEED/* * speedRate*/;

	D3DXVECTOR3 targetMove = input.moveDir;

	if (targetMove.x != 0.0f || targetMove.z != 0.0f)
	{
		D3DXVec3Normalize(&targetMove, &targetMove);

		targetMove *= moveSpeed;
	}
	else
	{
		targetMove = INIT_VEC3;
	}

	// 現在速度との補間（イージング）
	D3DXVECTOR3 currentMove = pPlayer->GetMove();

	currentMove.x += (targetMove.x - currentMove.x) * CPlayer::ACCEL_RATE;
	currentMove.z += (targetMove.z - currentMove.z) * CPlayer::ACCEL_RATE;

	// 補間後の速度をプレイヤーにセット
	pPlayer->SetMove(currentMove);

	// プレイヤーの位置取得
	D3DXVECTOR3 pos = pPlayer->GetPos();

	// 移動していなければ待機ステートに戻す
	if (!pPlayer->GetIsMoving())
	{
		// 待機状態
		m_pMachine->ChangeState<CPlayerStandState>();
	}
}
//=============================================================================
// 移動状態の終了処理
//=============================================================================
void CPlayerMoveState::OnExit(CPlayer* pPlayer)
{
	// モーションスピードを通常に戻す
	pPlayer->GetMotion()->SetMotionSpeedRate(CMotion::RESET_MOTION_RATE);
}