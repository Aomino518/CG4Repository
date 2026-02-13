#pragma once
#include "Matrix.h"
#include "CreateResorceUtils.h"
#include "ParticleManager.h"

class ParticleEmitter
{
public:
	ParticleEmitter(const std::string& groupName, uint32_t count, float frequency);
	void EmitOnce();
	void StartLoop();
	void StopLoop();
	void Update();

	bool GetIsLoop() { return isLoop_; }


	Transform transform_;
	uint32_t count_; // 1回の発生個数
	float frequency_; // 発生頻度

private:
	std::string groupName_;
	float frequencyTime_; // 発生タイマー
	const float kDeltaTime = 1.0f / 60.0f;
	bool isLoop_ = false; // 無限発生か
};