//=============================================================================
//
// ライト処理 [Light.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Light.h"
#include "Renderer.h"
#include "Manager.h"
#include "ObjectBillboard.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
int CLight::m_lightCount = 0;
CLight::LightInfo CLight::m_lights[MAX_LIGHTS] = {};
CObjectX* CLight::m_pObjectX = nullptr;

namespace LightParam
{
    namespace Point
    {
        constexpr float ATTENUATION_0   = 0.0f;
        constexpr float ATTENUATION_1   = 0.006f;
        constexpr float ATTENUATION_2   = 0.0f;
        constexpr float RANGE           = 130.0f;
    }

    namespace Spot
    {
        constexpr float RANGE           = 100.0f;
        constexpr float THETA           = 70.0f;
        constexpr float PHI             = 95.0f;
        constexpr float FALLOUT         = 1.0f;
        constexpr float ATTENUATION_0   = 1.0f;
        constexpr float ATTENUATION_1   = 0.003f;
        constexpr float ATTENUATION_2   = 0.0f;
    }
}                       

//=============================================================================
// コンストラクタ
//=============================================================================
CLight::CLight()
{
	// 値のクリア
}
//=============================================================================
// デストラクタ
//=============================================================================
CLight::~CLight()
{
	// ライトの終了処理
	Uninit();
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CLight::Init(void)
{
	m_lightCount = 0;

	for (int nCnt = 0; nCnt < MAX_LIGHTS; nCnt++)
	{
		m_lights[nCnt].enabled = false;
	}

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CLight::Uninit(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	for (int nCnt = 0; nCnt < MAX_LIGHTS; nCnt++)
	{
		if (m_lights[nCnt].enabled)
		{
			pDevice->LightEnable(nCnt, FALSE);
			m_lights[nCnt].enabled = false;
		}
	}

	m_lightCount = 0;
}
//=============================================================================
// 更新処理
//=============================================================================
void CLight::Update(void)
{
    // なし
}
//=============================================================================
// ライトの追加処理
//=============================================================================
int CLight::AddLight(D3DLIGHTTYPE type, const D3DXCOLOR& diffuse, const D3DXVECTOR3& direction, const D3DXVECTOR3& position)
{
    if (m_lightCount >= MAX_LIGHTS)
    {
        return -1;
    }

    // 名前空間LightParamの使用
    using namespace LightParam;

    int index = m_lightCount;
    LightInfo& lightInfo = m_lights[index];

    ZeroMemory(&lightInfo.light, sizeof(D3DLIGHT9));
    lightInfo.light.Type = type;
    lightInfo.light.Diffuse = diffuse;
    lightInfo.position = position;
    lightInfo.light.Position = position;

    D3DXVECTOR3 dir = direction;
    if (D3DXVec3Length(&dir) == 0.0f)
    {
        dir = D3DXVECTOR3(0, -1, 0); // デフォルト下向き
    }

    D3DXVec3Normalize(&lightInfo.direction, &dir);
    lightInfo.light.Direction = lightInfo.direction;

//#ifdef _DEBUG
//
//    // ライト用モデルの生成
//    m_pObjectX = CObjectX::Create("data/MODELS/light.x", position, lightInfo.light.Direction, D3DXVECTOR3(1.0f, 1.0f, 1.0f));
//
//#endif // DEBUG

    // ライトの種類に応じた設定
    if (type == D3DLIGHT_POINT)
    {
        lightInfo.light.Attenuation0    = Point::ATTENUATION_0;
        lightInfo.light.Attenuation1    = Point::ATTENUATION_1;
        lightInfo.light.Attenuation2    = Point::ATTENUATION_2;
        lightInfo.light.Range           = Point::RANGE;
    }
    else if (type == D3DLIGHT_SPOT)
    {
        lightInfo.light.Range           = Spot::RANGE;
        lightInfo.light.Theta           = D3DXToRadian(Spot::THETA);  // 明るく照らす範囲（中心）
        lightInfo.light.Phi             = D3DXToRadian(Spot::PHI);  // 減衰していく外側        
        lightInfo.light.Falloff         = Spot::FALLOUT;
        lightInfo.light.Attenuation0    = Spot::ATTENUATION_0;
        lightInfo.light.Attenuation1    = Spot::ATTENUATION_1;    // 緩やかな減衰
        lightInfo.light.Attenuation2    = Spot::ATTENUATION_2;
    }

    // デバイスの取得
    LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

    pDevice->SetLight(index, &lightInfo.light);
    pDevice->LightEnable(index, TRUE);

    lightInfo.enabled = true;

    // ライトのカウントを増やす
    m_lightCount++;

    return index;
}
//=============================================================================
// ライトの削除処理
//=============================================================================
void CLight::DeleteLight(int index)
{
    if (index >= 0 && index < MAX_LIGHTS && m_lights[index].enabled)
    {
        // デバイスの取得
        LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

        // ライトをオフにする
        pDevice->LightEnable(index, FALSE);
        m_lights[index].enabled = false;

        // ライトのカウントを減らす
        m_lightCount--;
    }
}
//=============================================================================
// 現在のライトを取得する処理
//=============================================================================
std::vector<CLight::LightInfo> CLight::GetCurrentLights(void)
{
    std::vector<LightInfo> backup;

    for (int nCnt = 0; nCnt < m_lightCount; nCnt++)
    {
        if (m_lights[nCnt].enabled)
        {
            LightInfo b;
            b.light = m_lights[nCnt].light;
            b.direction = m_lights[nCnt].direction;
            b.position = m_lights[nCnt].position;
            b.enabled = m_lights[nCnt].enabled;
            backup.push_back(b);
        }
    }
    return backup;
}
//=============================================================================
// 一時的にライトを退避する処理
//=============================================================================
void CLight::RestoreLights(const std::vector<LightInfo>& backup)
{
    Uninit(); // 一度全部削除してから

    for (const auto& b : backup)
    {
        int index = AddLight(b.light.Type,
            D3DXCOLOR(b.light.Diffuse),
            b.direction,
            b.position);
        // 元のD3DLIGHT9構造体を上書きして完全復元
        m_lights[index].light = b.light;
        m_lights[index].enabled = b.enabled;
    }
}
