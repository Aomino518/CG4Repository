#include "Sound.h"
#include <vector>
#include <assert.h>
#include "Logger.h"
#include <algorithm>

/// <summary>
/// 音声読み込み関数
/// </summary>
/// <param name="filename">ファイル名</param>
/// <returns>サウンドハンドル</returns>
SoundData Sound::SoundLoad(const char* filename)
{
	Logger::Write("Soundロード開始");
	std::string path(filename ? filename : "");
	std::string ext = ToLowerExt(path);

	SoundData soundData = {};

	Logger::Write("Soundがwavかmp3か判定開始");
	if (ext == ".wav") {
		Logger::Write("Soundはwav");
		soundData = SoundLoadWave(filename);
	}
	if (ext == ".mp3") {
		Logger::Write("Soundはmp3");
		std::wstring w = ToWide(filename);
		soundData = SoundLoadMP3(w.c_str());
	}

	return soundData;
}

/// <summary>
/// 音声再生関数
/// </summary>
/// <param name="soundData">サウンドハンドル</param>
/// <param name="isLoop">ループ判定</param>
/// <param name="volume">音量</param>
void Sound::SoundPlay(const SoundData& soundData, bool isLoop, float volume) {
	if (!soundCommon_) {
		return;
	}
	
	if (isPlaying_) {
		SoundStop();
	}

	HRESULT result;
	isLooping_ = isLoop;
	currentData_ = soundData;

	// 波形フォーマットを元にSourceVoiceの生成
	pSourceVoice_ = nullptr;
	result = soundCommon_->GetXAudio()->CreateSourceVoice(&pSourceVoice_, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.LoopCount = isLoop ? XAUDIO2_LOOP_INFINITE : 0;

	currentVolume_ = volume;
	// 音量設定
	pSourceVoice_->SetVolume(currentVolume_);

	// 波形データの再生
	result = pSourceVoice_->SubmitSourceBuffer(&buf);
	result = pSourceVoice_->Start();
	isPlaying_ = true;
}

// 音声データ解放
void Sound::SoundUnload(SoundData* soundData) {
	// バッファのメモリ解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

/// <summary>
/// 音声停止関数
/// </summary>
void Sound::SoundStop()
{
	if (pSourceVoice_) {
		pSourceVoice_->Stop();
		pSourceVoice_->FlushSourceBuffers();
		pSourceVoice_->DestroyVoice();
		pSourceVoice_ = nullptr;
	}
	isPlaying_ = false;
}

/// <summary>
/// 音声再再生関数
/// </summary>
void Sound::SoundRestart()
{
	if (!currentData_.pBuffer) {
		return;
	}

	SoundPlay(currentData_, isLooping_);
}

/// <summary>
/// 音量調整関数
/// </summary>
/// <param name="volume">音量</param>
void Sound::SetVolume(float volume)
{
	currentVolume_ = std::clamp(volume, 0.0f, 1.0f);
	if (pSourceVoice_) {
		pSourceVoice_->SetVolume(currentVolume_);
	}
}

std::wstring Sound::ToWide(const char* utf8) {
	if (!utf8) {
		return L"";
	}

	int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
	std::wstring w(wlen, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, w.data(), wlen);
	if (!w.empty() && w.back() == L'\0') {
		w.pop_back();
	}

	return w;
}

std::string Sound::ToLowerExt(const std::string& path) {
	auto pos = path.find_last_of('.');
	std::string ext = (pos == std::string::npos) ? "" : path.substr(pos);
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });
	return ext;
}

// wave読み込み
SoundData Sound::SoundLoadWave(const char* filename) {
	/*--ファイルオープン--*/
	// ファイル入力ストリームのインスタンス
	std::ifstream file;
	// .wavファイルをバイナリ―モードで開く
	file.open(filename, std::ios_base::binary);
	// ファイルオープン失敗を検出する
	assert(file.is_open());

	/*--.wavデータ読み込み--*/
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	// タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	// Formatチャンクの読み込み
	FormatChunk format = {};
	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		// 読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		// 再読み込み
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	// Dataチャンクのデータ部(波形データ)の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// WAVEファイルを閉じる
	file.close();

	/*--読み込んだ音声データをリターン--*/
	// returnするための音声データ
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

// mp3読み込み
SoundData Sound::SoundLoadMP3(const wchar_t* wpath)
{
	Logger::Write("SoundLoadMp3開始");
	SoundData soundData = {};
	if (!soundCommon_->GetIsMfStarted()) {
		return soundData;
	}

	Microsoft::WRL::ComPtr<IMFSourceReader> pMFSourceReader{ nullptr };
	MFCreateSourceReaderFromURL(wpath, nullptr, &pMFSourceReader);
	Logger::Write("MFCreateSourceReaderFromURL通った");

	// ストリーム選択
	HRESULT hr = pMFSourceReader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), FALSE);

	if (FAILED(hr)) {
		return soundData;
	}
	Logger::Write("MF_SOURCE_READER_ALL_STREAMS通った");

	hr = pMFSourceReader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), TRUE);

	if (FAILED(hr)) {
		return soundData;
	}
	Logger::Write("MF_SOURCE_READER_FIRST_AUDIO_STREAM通った");

	// ネイティブ型からch/rateを取っておく
	Microsoft::WRL::ComPtr<IMFMediaType> pMFMediaType{ nullptr };
	UINT32 ch = 2, rate = 48000;
	if (SUCCEEDED(pMFSourceReader->GetNativeMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, &pMFMediaType))) {
		UINT32 v = 0;
		if (SUCCEEDED(pMFMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &v))) {
			ch = v;
		}

		if (SUCCEEDED(pMFMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &v))) {
			rate = v;
		}
	}

	hr = MFCreateMediaType(&pMFMediaType);

	if (FAILED(hr)) {
		Logger::Write("MFCreateMediaTypeエラー");
		return soundData;
	}
	Logger::Write("MFCreateMediaType通った");

	pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	pMFMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, ch);
	pMFMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, rate);
	pMFMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
	pMFMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, (ch * 16) / 8);
	pMFMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, rate * (ch * 16) / 8);

	hr = pMFSourceReader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr, pMFMediaType.Get());
	
	if (FAILED(hr)) {
		Logger::Write("SetCurrentMediaTypeエラー");
		return soundData;
	}
	Logger::Write("SetCurrentMediaType通った");

	Microsoft::WRL::ComPtr<IMFMediaType> finalType;
	hr = pMFSourceReader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &finalType);
	
	if (FAILED(hr)) {
		Logger::Write("GetCurrentMediaTypeエラー");
		return soundData;
	}
	Logger::Write("GetCurrentMediaType通った");

	WAVEFORMATEX* waveFormat{ nullptr };
	hr = MFCreateWaveFormatExFromMFMediaType(finalType.Get(), &waveFormat, nullptr);

	if (FAILED(hr) || waveFormat == nullptr) {
		Logger::Write("waveFormatがnullptrかMFCreateWaveFormatExFromMFMediaTypeエラー");
		return soundData;
	}

	std::vector<BYTE> mediaData;

	while (true) {
		Microsoft::WRL::ComPtr<IMFSample> pMFSample{ nullptr };
		DWORD dwStreamsFlags{ 0 };
		pMFSourceReader->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, nullptr, &dwStreamsFlags, nullptr, &pMFSample);

		if (dwStreamsFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			break;
		}

		Microsoft::WRL::ComPtr<IMFMediaBuffer> pMFMediaBuffer{ nullptr };
		pMFSample->ConvertToContiguousBuffer(&pMFMediaBuffer);

		BYTE* pBuffer{ nullptr };
		DWORD cbCurrentLength = 0, maxLength = 0;
		pMFMediaBuffer->Lock(&pBuffer, &maxLength, &cbCurrentLength);

		if (cbCurrentLength) {
			size_t old = mediaData.size();
			mediaData.resize(old + cbCurrentLength);
			std::memcpy(mediaData.data() + old, pBuffer, cbCurrentLength);
		}

		//mediaData.resize(static_cast<size_t>(mediaData.size()) + static_cast<size_t>(cbCurrentLength));
		//memcpy(mediaData.data() + mediaData.size() - static_cast<size_t>(cbCurrentLength), pBuffer, static_cast<size_t>(cbCurrentLength));

		pMFMediaBuffer->Unlock();
	}

	BYTE* heap = new BYTE[mediaData.size()];
	memcpy(heap, mediaData.data(), mediaData.size());

	soundData.wfex = *waveFormat;
	soundData.pBuffer = heap;
	soundData.bufferSize = static_cast<unsigned int>(mediaData.size());

	CoTaskMemFree(waveFormat);

	return soundData;
}