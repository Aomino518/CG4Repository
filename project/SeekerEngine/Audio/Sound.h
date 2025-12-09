#pragma once
#include "SoundCommon.h"

class Sound
{
public:
	void SetCommon(SoundCommon* soundCommon) { soundCommon_ = soundCommon; }

	SoundData SoundLoad(const char* filename);
	void SoundPlay(const SoundData& soundData, bool isLoop, float volume = 1.0f);
	void SoundUnload(SoundData* soundData);
	void SoundStop();
	void SoundRestart();
	void SetVolume(float volume);

private:
	// 内部用
	static std::wstring ToWide(const char* utf8);
	static std::string  ToLowerExt(const std::string& path);
	SoundData SoundLoadMP3(const wchar_t* wpath);
	SoundData SoundLoadWave(const char* filename);

	SoundCommon* soundCommon_ = nullptr;
	IXAudio2SourceVoice* pSourceVoice_ = nullptr;
	
	bool isLooping_ = false;
	bool isPlaying_ = false;
	float currentVolume_ = 1.0f;
	SoundData currentData_ = {};
};

