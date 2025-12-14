#include "ParticleManager.h"

ParticleManager* ParticleManager::instance_ = nullptr;

ParticleManager* ParticleManager::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new ParticleManager();
	}
	return instance_;
}

void ParticleManager::Init(Particle3DCommon* particleCommon, Graphics* graphics, DxcCompiler& dxcCompiler, ID3D12RootSignature* rootSignature)
{
	particleCommon_ = particleCommon;
	particleCommon_->Init(graphics, dxcCompiler, rootSignature);
}

void ParticleManager::Update(CameraManager* cameraManager)
{
	particleCommon_->UpdateEmitter();
	particleCommon_->UpdateInstanceData(cameraManager);
}

void ParticleManager::Draw()
{
	particleCommon_->Draw();
}

void ParticleManager::SetTexture(uint32_t textureId)
{
	particleCommon_->SetTexture(textureId);
}

void ParticleManager::Emit(const Emitter& emitter, std::mt19937& randomEngine)
{
	particleCommon_->Emit(emitter, randomEngine);
}
