//=============================================================================
//
// キャラクター処理 [Character.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _CHARACTER_H_
#define _CHARACTER_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Object.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CGuage;

//*****************************************************************************
// キャラクタークラス
//*****************************************************************************
class CCharacter : public CObject
{
public:
    CCharacter(int nPriority = PRIORITY::CHARACTER);
    virtual ~CCharacter();

    virtual HRESULT Init(void) = 0;
    virtual void Uninit(void) = 0;
    virtual void Update(void) = 0;
    virtual void Draw(void) = 0;

    // 当たり判定の生成
    void CreatePhysics(float radius, float height, btScalar mass);

    // Physicsの破棄
    void ReleasePhysics(void);

    // 当たり判定の位置更新
    void UpdateCollider(D3DXVECTOR3 offset);

    // 向き補間処理
    void UpdateRotation(float fInterpolationSpeed);

    // HPゲージの設定処理
    void SetGuages(D3DXVECTOR3 pos, D3DXCOLOR colHP, D3DXCOLOR colBack, float fWidth, float fHeight);

    // ダメージ処理
    virtual void Damage(float fDamage);

    // 回復処理
    void Heal(float fHealAmount);

    //*****************************************************************************
    // flagment関数
    //*****************************************************************************
    bool IsDead(void) { return m_isDead; }

    //*****************************************************************************
    // setter関数
    //*****************************************************************************
    void SetHp(float fHp) { m_fHp = fHp; m_fMaxHp = m_fHp; }
    void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
    void SetRot(D3DXVECTOR3 rot) { m_rot = rot; }
    void SetSize(D3DXVECTOR3 size) { m_size = size; }
    void SetRotDest(const D3DXVECTOR3& rotDest) { m_rotDest = rotDest; }
    void SetMove(D3DXVECTOR3 move);

    //*****************************************************************************
    // getter関数
    //*****************************************************************************
    D3DXVECTOR3 GetPos(void) { return m_pos; }
    D3DXVECTOR3 GetRot(void) { return m_rot; };
    const D3DXVECTOR3& GetRotDest(void) const { return m_rotDest; }
    D3DXVECTOR3 GetSize(void) { return m_size; }
    D3DXVECTOR3 GetMove(void) const { return m_move; }
    float GetHp(void) const { return m_fHp; }
    float GetMaxHp(void) const { return m_fMaxHp; }
    btScalar GetRadius(void) const { return m_radius; }
    btScalar GetHeight(void) const { return m_height; }
    btRigidBody* GetRigidBody(void) const { return m_pRigidBody; }						// RigidBodyの取得
    btCollisionShape* GetCollisionShape(void) { return m_pShape; }
    D3DXVECTOR3 GetColliderPos(void) const { return m_colliderPos; }
    virtual int GetCollisionFlags(void) const { return 0; }// デフォルトはフラグなし

private:
    CGuage*             m_pHpGuage;		// 緑ゲージ
    CGuage*             m_pBackGuage;	// 赤ゲージ（遅れて追従）
    CGuage*             m_pFrame;		// 枠
    btRigidBody*        m_pRigidBody;	// 剛体へのポインタ
    btCollisionShape*   m_pShape;	    // 当たり判定の形へのポインタ
    D3DXVECTOR3         m_pos;			// 位置
    D3DXVECTOR3         m_colliderPos;	// カプセルコライダーの位置
    D3DXVECTOR3         m_rot;			// 向き
    D3DXVECTOR3         m_rotDest;		// 向き
    D3DXVECTOR3         m_size;			// サイズ
    D3DXVECTOR3         m_move;			// 移動量
    btScalar            m_radius;		// カプセルコライダーの半径
    btScalar            m_height;		// カプセルコライダーの高さ
    float               m_fHp;          // HP
    float               m_fMaxHp;       // HP最大量
    bool                m_isDead;       // 死んだかどうか

};

#endif