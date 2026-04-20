//=============================================================================
//
// キャラクターマネージャー処理 [CharacterManager.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _CHARACTERMANAGER_H_
#define _CHARACTERMANAGER_H_


//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CCharacter;

//*****************************************************************************
// キャラクターマネージャークラス
//*****************************************************************************
class CCharacterManager
{
public:
    // インスタンスの取得
    static CCharacterManager& GetInstance(void)
    {
        static CCharacterManager instance;
        return instance;
    }

    // キャラクター追加処理
    void AddCharacter(CCharacter* pChar);

    // 単体キャラクターの取得処理
    template <class characterType>
    characterType* GetCharacter(void)
    {
        for (const auto& c : m_characters)
        {
            if (auto casted = dynamic_cast<characterType*>(c))
            {
                return casted;
            }
        }
        return nullptr;
    }

    // 複数体キャラクターの取得処理
    template <class characterType>
    std::vector<characterType*> GetCharacters(void)
    {
        std::vector<characterType*> result;
        for (auto* c : m_characters)
        {
            if (auto* casted = dynamic_cast<characterType*>(c))
            {
                result.push_back(casted);
            }
        }
        return result;
    }

    void Init(void);
    void Destroy(void);
    void Update(void);
    void Draw(void);

private:
    CCharacterManager() = default;
    ~CCharacterManager() = default;

    CCharacterManager(const CCharacterManager&) = delete;
    CCharacterManager& operator=(const CCharacterManager&) = delete;

    std::vector<CCharacter*> m_characters;// キャラクターのリスト
};

#endif

