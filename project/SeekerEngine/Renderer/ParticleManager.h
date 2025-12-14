#pragma once
#include "Graphics.h"
#include "DxcCompiler.h"
#include <random>
#include <numbers>
#include "Particle3DCommon.h"
#include "CreateResorceUtils.h"
#include "CameraManager.h"

struct ParticleGroup {
    // テクスチャ関連
    uint32_t textureIndex = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle{};
    // パーティクル本体
    std::list<Particle> particles;
    // インスタンシング
    Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource;
    uint32_t instanceCount = 0;
    ParticleForGPU* instanceData = nullptr;
};

class ParticleManager
{
public:
	static ParticleManager* GetInstance();

	void Init(Particle3DCommon* particleCommon, Graphics* graphics, DxcCompiler& dxcCompiler, ID3D12RootSignature* rootSignature);

	void Update(CameraManager* cameraManager);

	void Draw();

    void SetTexture(uint32_t textureId);

    void Emit(const Emitter& emitter, std::mt19937& randomEngine);

private:
	static ParticleManager* instance_;

    ParticleManager() = default;
    ~ParticleManager() = default;
    ParticleManager(const ParticleManager&) = delete;
    ParticleManager& operator=(const ParticleManager&) = delete;

	Particle3DCommon* particleCommon_;
};

