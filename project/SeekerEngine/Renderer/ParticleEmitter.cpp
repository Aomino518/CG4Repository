#include "ParticleEmitter.h"

ParticleEmitter::ParticleEmitter(const std::string& groupName, uint32_t count, float frequency) :
groupName_(groupName),
count_(count),
frequency_(frequency),
frequencyTime_(0.0f)
{
	transform_.translate = { 0.0f, 0.0f, 0.0f };
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	transform_.scale = { 1.0f, 1.0f, 1.0f };
}

void ParticleEmitter::EmitOnce()
{
	ParticleManager::GetInstance()->Emit(groupName_, transform_.translate, count_);
}

void ParticleEmitter::StartLoop()
{
	isLoop_ = true;
	frequencyTime_ = 0.0f;
}

void ParticleEmitter::StopLoop()
{
	isLoop_ = false;
}

void ParticleEmitter::Update()
{
	if (isLoop_) {
		frequencyTime_ += kDeltaTime;

		while (frequency_ <= frequencyTime_) {
			ParticleManager::GetInstance()->Emit(groupName_, transform_.translate, count_);
			frequencyTime_ -= frequency_;
		}
	}
}