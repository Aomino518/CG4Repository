#include "SoundCommon.h"

void SoundCommon::Init()
{
	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	hr = xAudio2_->CreateMasteringVoice(&masterVoice_);

	hr = MFStartup(MF_VERSION, 0);
	if (SUCCEEDED(hr)) {
		mfStarted_ = true;
	}
}

void SoundCommon::Shutdown()
{
	if (masterVoice_) {
		masterVoice_->DestroyVoice();
		masterVoice_ = nullptr;
	}

	// XAudio2解放
	xAudio2_.Reset();

	if (mfStarted_) {
		MFShutdown();
		mfStarted_ = false;
	}
}
