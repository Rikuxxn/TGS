//=============================================================================
//
// サウンド処理 [Sound.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _SOUND_H_// このマクロ定義がされていなかったら
#define _SOUND_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// サウンドクラス
//*****************************************************************************
class CSound
{
public:
	CSound();
	~CSound();

	//*************************************************************************
	// サウンド一覧
	//*************************************************************************
	typedef enum
	{
		SOUND_LABEL_GAMEBGM = 0,	// ゲームBGM
		SOUND_LABEL_PAUSE,
		SOUND_LABEL_SELECT,
		SOUND_LABEL_ENTER,
		SOUND_LABEL_MAX,
	} SOUND_LABEL;

	HRESULT Init(HWND hWnd);
	void Uninit(void);

	void PauseAll(void);
	void ResumeAll(void);

	void Stop(int instanceId);			// インデックス指定での停止(3Dサウンド)
	void StopByLabel(SOUND_LABEL label);// ラベル指定での停止(2Dサウンド)
	void Stop(void);
	int Play(SOUND_LABEL label);
	int Play3D(SOUND_LABEL label,D3DXVECTOR3 soundPos,float minDistance,float maxDistance);

	void UpdateListener(D3DXVECTOR3 pos);
	void UpdateSoundPosition(int instanceId, D3DXVECTOR3 pos);

private:
	static constexpr int MAX_SIMULTANEOUS_PLAY = 2;	// 最大同時再生数


	//*************************************************************************
	// 一つのサウンド再生インスタンス構造体
	//*************************************************************************
	struct SoundInstance
	{
		int id = -1;
		IXAudio2SourceVoice* pSourceVoice = nullptr;
		X3DAUDIO_EMITTER emitter = {};
		SOUND_LABEL label;
		bool active = false;
		float minDistance = 0.0f, maxDistance = 0.0f;		// 最小距離と最大距離（距離減衰の範囲）
	};

	//*************************************************************************
	// サウンドデータ構造体
	//*************************************************************************
	struct SoundData
	{
		BYTE* pAudioData = nullptr;
		DWORD audioBytes = 0;
		WAVEFORMATEXTENSIBLE wfx = {};
	};

	struct SOUNDINFO
	{
		const char* pFilename;
		int loopCount;
	};

private:
	//*************************************************************************
	// XAudio
	//*************************************************************************
	IXAudio2* m_pXAudio2;									// XAudio2オブジェクトへのインターフェイス
	IXAudio2MasteringVoice* m_pMasteringVoice;				// マスターボイス


	//*************************************************************************
	// X3DAudio
	//*************************************************************************
	X3DAUDIO_HANDLE m_X3DInstance;							// X3DAudio インスタンス
	X3DAUDIO_LISTENER m_Listener;							// リスナー（プレイヤーの位置）

	//*************************************************************************
	// サウンドの情報
	//*************************************************************************
	SOUNDINFO m_aSoundInfo[SOUND_LABEL_MAX] =
	{
		{"data/BGM/gameBGM.wav", -1},			// ゲームBGM
		{"data/SE/menu.wav", 0},				// ポーズSE
		{"data/SE/select.wav", 0},				// 選択SE
		{"data/SE/enter.wav", 0},				// 決定SE
	};

	SoundData m_SoundData[SOUND_LABEL_MAX];
	std::vector<SoundInstance> m_Instances;	// インスタンス管理
	int m_nextInstanceId;

private:
	HRESULT CreateAndStartVoice(SOUND_LABEL label, SoundInstance& inst);
	HRESULT LoadWave(SOUND_LABEL label);
	HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD* pChunkSize, DWORD* pChunkDataPosition);
	HRESULT ReadChunkData(HANDLE hFile, void* pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset);
	void	CalculateCustomPanning(SoundInstance& inst, FLOAT32* matrix);

};

#endif