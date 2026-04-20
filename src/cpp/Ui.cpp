//=============================================================================
//
// UI処理 [Ui.h]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Ui.h"
#include "Manager.h"
#include "Result.h"
#include "Easing.h"
#include "MathConst.h"

namespace RankRange
{
    constexpr int   S_NUM       = 8;
    constexpr int   A_NUM_MIN   = 5;
    constexpr int   A_NUM_MAX   = 7;
    constexpr int   B_NUM_MIN   = 3;
    constexpr int   B_NUM_MAX   = 4;
    constexpr float A_RATE_MIN  = 0.8f;
    constexpr float A_RATE_MAX  = 0.9f;
    constexpr float B_RATE_MIN  = 0.6f;
    constexpr float B_RATE_MAX  = 0.8f;
    constexpr float C_RATE_MIN  = 0.6f;
    constexpr float C_RATE_MAX  = 0.8f;
    constexpr float DEC_RATE    = 0.01f;// 低下割合(1%)
}

//=============================================================================
// UIマネージャーのインスタンス生成
//=============================================================================
CUIManager* CUIManager::GetInstance(void)
{
    static CUIManager inst;
    return &inst;
}
//=============================================================================
// UIマネージャーの初期化処理
//=============================================================================
HRESULT CUIManager::Init(void)
{
    // リストを空にする
    m_uiList.clear();
    m_uiMap.clear();

    return S_OK;
}
//=============================================================================
// UIマネージャーの終了処理
//=============================================================================
void CUIManager::Uninit(void)
{
    for (auto ui : m_uiList)
    {
        if (ui)
        {
            ui->Uninit();
            delete ui;
        }
    }

    // リストを空にする
    m_uiList.clear();
    m_uiMap.clear();
}
//=============================================================================
// UIマネージャーの更新処理
//=============================================================================
void CUIManager::Update(void)
{
    for (auto ui : m_uiList)
    {
        if (ui && ui->IsVisible())
        {
            ui->Update();
        }
    }
}
//=============================================================================
// UIマネージャーの描画処理
//=============================================================================
void CUIManager::Draw(void)
{
    for (auto ui : m_uiList)
    {
        // 表示がtrueのUIのみ描画
        if (ui && ui->IsVisible())
        {
            ui->Draw();
        }
    }
}
//=============================================================================
// UI追加
//=============================================================================
void CUIManager::AddUI(const std::string& name, CUIBase* ui)
{
    if (!ui)
    {
        return;
    }

    m_uiList.push_back(ui);
    m_uiMap[name] = ui;
}
//=============================================================================
// 登録した名前でUIを取得
//=============================================================================
CUIBase* CUIManager::GetUI(const std::string& name)
{
    auto it = m_uiMap.find(name);

    if (it != m_uiMap.end())
    {
        return it->second;
    }

    return nullptr;
}


//=============================================================================
// コンストラクタ
//=============================================================================
CUIBase::CUIBase(int nPriority) : CObject2D(nPriority)
{
	// 値のクリア
    memset(m_szPath, 0, sizeof(m_szPath));  // ファイルパス
    m_nIdxTexture   = 0;                    // テクスチャインデックス
    m_bVisible      = true;                 // 表示フラグ
    m_parent        = nullptr;              // 親ポインタ
    m_alpha         = 0.0f;                 // アルファ値
    m_fadeSpeed     = 0.0f;                 // フェードスピード
    m_fadeMode      = FadeMode::None;       // フェードモード
    m_slideMode     = SlideMode::None;      // スライドモード
    m_slideT        = 0.0f;                 // スライド割合
    m_slideSpeed    = 0.0f;                 // スライドスピード
    m_useLayout     = false;                // レイアウトの使用フラグ
    m_layoutPos     = INIT_VEC3;            // レイアウト用位置
    m_slideOffset   = INIT_VEC3;            // オフセット位置
}
//=============================================================================
// デストラクタ
//=============================================================================
CUIBase::~CUIBase()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CUIBase* CUIBase::Create(float x, float y, D3DXCOLOR col, float width, float height)
{
    CUIBase* pUi = new CUIBase;

    // nullptrだったら
    if (pUi == nullptr)
    {
        return nullptr;
    }

    pUi->SetPos(D3DXVECTOR3(x, y, 0.0f));
    pUi->SetCol(col);
    pUi->SetSize(width, height);

    // 初期化失敗時
    if (FAILED(pUi->Init()))
    {
        return nullptr;
    }

    return pUi;
}
//=============================================================================
// 初期化処理処理
//=============================================================================
HRESULT CUIBase::Init(void)
{
    // テクスチャの取得
    m_nIdxTexture = CManager::GetTexture()->RegisterDynamic(m_szPath);

    // 2Dオブジェクトの初期化処理
    CObject2D::Init();

    return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CUIBase::Uninit(void)
{
    for (auto child : m_children)
    {
        child->Uninit();
        delete child;
    }

    // リストを空にする
    m_children.clear();
}
//=============================================================================
// 更新処理
//=============================================================================
void CUIBase::Update(void)
{
    // レイアウトを使用する場合
    if (m_useLayout)
    {
        // バックバッファサイズの取得
        float sw = (float)CManager::GetRenderer()->GetBackBufferWidth();
        float sh = (float)CManager::GetRenderer()->GetBackBufferHeight();

        float w = sw * m_layout.widthRate;
        float h = sh * m_layout.heightRate;

        m_layoutPos.x = sw * m_layout.anchorX - w * CMathConstant::HALF;
        m_layoutPos.y = sh * m_layout.anchorY - h * CMathConstant::HALF;
        m_layoutPos.z = 0.0f;

        SetSize(w, h);
    }

    // フェード制御
    if (m_fadeMode == FadeMode::FadeIn)
    {
        m_alpha += m_fadeSpeed;

        if (m_alpha >= 1.0f)
        {
            m_alpha = 1.0f;
            m_fadeMode = FadeMode::None;
            m_bVisible = true;
        }

        ApplyAlpha();
    }
    else if (m_fadeMode == FadeMode::FadeOut)
    {
        m_alpha -= m_fadeSpeed;

        if (m_alpha <= 0.0f)
        {
            m_alpha = 0.0f;
            m_fadeMode = FadeMode::None;
            m_bVisible = false; // 完全に消えたら非表示状態に
        }

        ApplyAlpha();
    }

    // スライド制御
    if (m_slideMode != SlideMode::None)
    {
        m_slideT += m_slideSpeed;

        if (m_slideT >= 1.0f)
        {
            m_slideT = 1.0f;
            m_slideMode = SlideMode::None;

            if (!m_bVisible)
            {
                // SlideOut完了後に非表示にする場合
                // m_bVisible = false;
            }
        }

        // イージング
        float t = CEasing::Ease(0.0f, 1.0f, m_slideT, CEasing::EaseOutQuint);

        m_slideOffset =
            m_slideStartPos + (m_slideEndPos - m_slideStartPos) * t;
    }

    // 最終座標を合成して更新
    SetPos(m_layoutPos + m_slideOffset);

    // 2Dオブジェクトの更新処理
    CObject2D::Update();

    for (auto child : m_children)
    {
        child->Update();
    }
}
//=============================================================================
// 描画処理
//=============================================================================
void CUIBase::Draw(void)
{
    // デバイスの取得
    LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

    // テクスチャの設定
    pDevice->SetTexture(0, CManager::GetTexture()->GetAddress(m_nIdxTexture));

    // 2Dオブジェクトの描画処理
    CObject2D::Draw();

    for (auto child : m_children)
    {
        child->Draw();
    }
}
//=============================================================================
// ファイルパス設定処理
//=============================================================================
void CUIBase::SetPath(const char* path)
{
    if (path == nullptr)
    {
        path = " ";
    }

    strcpy_s(m_szPath, MAX_PATH, path);
}
//=============================================================================
// アンカーポイント設定処理
//=============================================================================
void CUIBase::SetAnchor(float x, float y)
{
    m_useLayout = true;// レイアウト使用
    m_layout.anchorX = x;
    m_layout.anchorY = y;
}
//=============================================================================
// アルファ値適用処理
//=============================================================================
void CUIBase::ApplyAlpha(void)
{
    SetCol(D3DXCOLOR(1.0f, 1.0f, 1.0f, m_alpha));
}
//=============================================================================
// 表示・非表示処理(即時)
//=============================================================================
void CUIBase::Show(void)
{
    m_bVisible = true;
    m_alpha = 1.0f;
    m_fadeMode = FadeMode::None;
    ApplyAlpha();
}
void CUIBase::Hide(void)
{
    m_bVisible = false;
    m_alpha = 0.0f;
    m_fadeMode = FadeMode::None;
    ApplyAlpha();
}
//=============================================================================
// 表示・非表示処理(フェード)
//=============================================================================
void CUIBase::FadeIn(float duration)
{
    //m_bVisible = true;

    // フェード開始時にアルファを0にする
    m_alpha = 0.0f;
    ApplyAlpha();

    m_fadeMode = FadeMode::FadeIn;
    m_fadeSpeed = 1.0f / duration;
}
void CUIBase::FadeOut(float duration)
{
    // フェード開始時にアルファを1にする
    m_alpha = 1.0f;
    ApplyAlpha();

    m_fadeMode = FadeMode::FadeOut;
    m_fadeSpeed = 1.0f / duration;
}
//=============================================================================
// スライドイン・アウト処理
//=============================================================================
void CUIBase::SlideIn(const D3DXVECTOR3& from, const D3DXVECTOR3& to, float duration)
{
    m_bVisible = true;

    m_slideStartPos = from;
    m_slideEndPos   = to;
    m_slideT        = 0.0f;
    m_slideSpeed    = 1.0f / duration;
    m_slideMode     = SlideMode::SlideIn;
}
void CUIBase::SlideOut(const D3DXVECTOR3& to, float duration)
{
    m_slideStartPos = m_slideOffset;
    m_slideEndPos   = to;
    m_slideT        = 0.0f;
    m_slideSpeed    = 1.0f / duration;
    m_slideMode     = SlideMode::SlideOut;
}
//=============================================================================
// マウスカーソル判定
//=============================================================================
bool CUIBase::IsMouseOver(void)
{
    if (!m_bVisible)
    {
        return false;
    }

    // マウス位置を取得（スクリーン座標）
    POINT cursorPos;
    GetCursorPos(&cursorPos);

    // ウィンドウハンドルを取得
    HWND hwnd = GetActiveWindow();

    // スクリーン座標をクライアント座標に変換
    ScreenToClient(hwnd, &cursorPos);

    // UI の範囲を計算
    float left = GetPos().x - GetWidth();
    float right = GetPos().x + GetWidth();
    float top = GetPos().y - GetHeight();
    float bottom = GetPos().y + GetHeight();

    return (cursorPos.x >= left && cursorPos.x <= right &&
        cursorPos.y >= top && cursorPos.y <= bottom);
}


//=============================================================================
// UIテクスチャのコンストラクタ
//=============================================================================
CUITexture::CUITexture()
{
    // 値のクリア
    m_isUVDirty = false;
}
//=============================================================================
// UIテクスチャのデストラクタ
//=============================================================================
CUITexture::~CUITexture()
{
    // なし
}
//=============================================================================
// UIテクスチャ生成処理(位置割合指定)
//=============================================================================
CUITexture* CUITexture::Create(const char* path, float anchorX, float anchorY, D3DXCOLOR col, float widthRate, float heightRate)
{
    CUITexture* pUi = new CUITexture;

    // nullptrだったら
    if (pUi == nullptr)
    {
        return nullptr;
    }

    pUi->SetPath(path);
    pUi->SetAnchor(anchorX, anchorY);
    pUi->SetCol(col);
    pUi->SetSizeRate(widthRate, heightRate);

    // 初期化失敗時
    if (FAILED(pUi->Init()))
    {
        return nullptr;
    }

    return pUi;
}
//=============================================================================
// UIテクスチャ初期化処理
//=============================================================================
HRESULT CUITexture::Init(void)
{
    // UIベースの初期化
    CUIBase::Init();

    return S_OK;
}
//=============================================================================
// UIテクスチャ終了処理
//=============================================================================
void CUITexture::Uninit(void)
{
    // UIベースの終了
    CUIBase::Uninit();
}
//=============================================================================
// UIテクスチャ更新処理
//=============================================================================
void CUITexture::Update(void)
{
    // 名前空間RankRangeの使用
    using namespace RankRange;

    // UIベースの更新
    CUIBase::Update();

    if (!m_isUVDirty)
    {
        return;
    }

    //float uvLeft = 0.0f;
    //int treasureCount = CResult::GetTreasureCount();
    //int soundCount = CResult::GetSoundCount();

    //// 次に音発生数に応じてレートを落とす
    //float EvaluationRate = 1.0f - soundCount * DEC_RATE;// 1%ずつ低下
    //EvaluationRate = std::max(EvaluationRate, 0.0f); // 最大0%まで下がる

    //// 埋蔵金の個数で評価した後に音発生数でレートを下げて、最終決定する
    //if (treasureCount >= S_NUM)// Sランク
    //{// 埋蔵金8個以上
    //    uvLeft = 0.0f;   // S

    //    if (EvaluationRate < A_RATE_MAX && EvaluationRate >= A_RATE_MIN)
    //    {// レートが90%より小さい かつ 80%以上
    //        uvLeft = UV_1;  // A
    //    }
    //    else if (EvaluationRate < B_RATE_MAX && EvaluationRate >= B_RATE_MIN)
    //    {// レートが80%より小さい かつ 60%以上
    //        uvLeft = UV_2;   // B
    //    }
    //    else if(EvaluationRate <= C_RATE_MIN)
    //    {// レートが50%以下
    //        uvLeft = UV_3;  // C
    //    }
    //}
    //else if (treasureCount >= A_NUM_MIN && treasureCount <= A_NUM_MAX)// Aランク
    //{// 埋蔵金5個以上7個以下
    //    uvLeft = UV_1;  // A

    //    if (EvaluationRate < B_RATE_MAX && EvaluationRate >= B_RATE_MIN)
    //    {// レートが80%より小さい かつ 60%以上
    //        uvLeft = UV_2;   // B
    //    }
    //    else if(EvaluationRate <= C_RATE_MIN)
    //    {// レートが60%以下
    //        uvLeft = UV_3;  // C
    //    }
    //}
    //else if (treasureCount >= B_NUM_MIN && treasureCount <= B_NUM_MAX)// Bランク
    //{// 埋蔵金3個以上4個以下
    //    uvLeft = UV_2;   // B

    //    if (EvaluationRate <= C_RATE_MAX)
    //    {// レートが80%以下
    //        uvLeft = UV_3;  // C
    //    }
    //}
    //else
    //{// それ以外
    //    uvLeft = UV_3;  // C
    //}

    //// テクスチャUV移動処理
    //CObject2D::MoveTexUV(uvLeft, 0.0f, UV_1, 1.0f);
}
//=============================================================================
// UIテクスチャ描画処理
//=============================================================================
void CUITexture::Draw(void)
{
    // UIベースの描画
    CUIBase::Draw();
}