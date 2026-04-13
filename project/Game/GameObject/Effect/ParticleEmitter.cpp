#include "ParticleEmitter.h"
#include "DebugDraw.h"

ParticleEmitter::ParticleEmitter(const std::string& groupName, 
	const ParticleConfig& config,
	uint32_t count,
	float frequency)
	: groupName_(groupName),
	config_(config),
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
	ParticleManager::GetInstance()->Emit(
		groupName_, 
		config_,
		transform_.translate,
		count_);
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
			ParticleManager::GetInstance()->Emit(
				groupName_, 
				config_,
				transform_.translate,
				count_);
			frequencyTime_ -= frequency_;
		}
	}
}

void ParticleEmitter::SetSpawnShapeBox(const Vector3& min, const Vector3& max)
{
	config_.shape = SpawnShape::Box;
	config_.boxMin = min;
	config_.boxMax = max;
}

void ParticleEmitter::SetSpawnShapeSphere(float radius)
{
	config_.shape = SpawnShape::Sphere;
	config_.sphereRadius = radius;
}

void ParticleEmitter::DrawDebug() {
#ifdef _DEBUG
	DebugDraw::DrawAABB(transform_.translate, localField_.GetAABB(), Vector4(0.2f, 0.6f, 1.0f, 1.0f), DebugDrawMode::Wireframe);
#endif
}