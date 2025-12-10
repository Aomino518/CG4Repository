#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "Graphics.h"
#include "DxcCompiler.h"
#include "InputLayout.h"
#include "PsoBuilder.h"
#include "BlendStateUtils.h"
#include <unordered_map>
#include "CreateResorceUtils.h"
#include "Matrix.h"
#include "SrvManager.h"
#include "CameraManager.h"
#include "DebugCamera.h"
#include "TextureManager.h"
#include <random>

class Particle3DCommon
{
public:
	void Init(Graphics* graphics, DxcCompiler& dxcCompiler, ID3D12RootSignature* rootSignature);

	void DrawCommon();

	void Draw();

	void SetBlendMode(BlendMode mode);

	void RebuildPso();

	void UpdateInstanceData(CameraManager* cameraManager);

	void SetTexture(uint32_t textureId) {
		textureIndex_ = textureId;
		textureSrvHandleGPU_ = TextureManager::GetInstance()->GetGPUHandle(textureId);
	}

	Particle MakeNewParticle(std::mt19937& randomEngine);

private:
	void CreateGraphicsPipeline(Graphics* graphics, DxcCompiler& dxcCompiler);
	// 板ポリ生成関数
	void CreatePlaneModel();
	void CreateInstanceResource();

	Graphics* graphics_ = nullptr;
	Camera* camera_ = nullptr;
	DebugCamera* debugCamera_ = nullptr;
	CameraManager* cameraManager_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> psoParticle3D_;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;

	Microsoft::WRL::ComPtr<IDxcBlob> vsBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> psBlob_;

	D3D12_BLEND_DESC blendDesc_{};
	BlendMode mode_ = kBlendModeAdd;
	std::unordered_map<BlendMode, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psoCache_;

	// リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	D3D12_INDEX_BUFFER_VIEW  ibView_{};

	static constexpr uint32_t kNumMaxInstance_ = 100;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
	ParticleForGPU* instancingData_ = nullptr;

	uint32_t instanceSrvIndex_ = 0;
	// テクスチャ番号
	uint32_t textureIndex_ = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Material* materialData = nullptr;

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{};

	Particle particle_[kNumMaxInstance_];

	const float kDeltaTime = 1.0f / 60.0f;
	uint32_t numInstance = 0;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};
};

