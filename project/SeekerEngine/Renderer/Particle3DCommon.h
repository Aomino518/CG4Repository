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
#include <numbers>

struct Emitter {
	Transform transform;   // 発生位置・回転・拡縮
	uint32_t count;        // 1回の発生で何個出すか
	float frequency;       // 発生頻度
	float frequencyTime;   // 発生タイマー
};

struct AccelerationField {
	Vector3 acceleration;
	AABB area;
};

class Particle3DCommon
{
public:
	void Init(Graphics* graphics, DxcCompiler& dxcCompiler, ID3D12RootSignature* rootSignature);

	void DrawCommon();

	void Draw();

	void SetBlendMode(BlendMode mode);

	void RebuildPso();

	void UpdateInstanceData(CameraManager* cameraManager);

	// Getter関数
	bool GetUseBillboard() { return useBillboard_; }
	bool GetUseField() { return useField_; }
	BlendMode GetBlendMode() { return mode_; }
	uint32_t GetkNumMaxInstance() { return kNumMaxInstance_; }
	uint32_t GetAliveCount() const { return static_cast<uint32_t>(particles_.size()); }
	const std::list<Particle>& GetPatricles() const { return particles_; }
	Emitter& GetEmitter() { return emitter_; }
	AccelerationField& GetField() { return field_; }

	// Setter関数
	void SetTexture(uint32_t textureId) {
		textureIndex_ = textureId;
		textureSrvHandleGPU_ = TextureManager::GetInstance()->GetGPUHandle(textureId);
	}

	void SetUseBillboard(bool useBillboard) { this->useBillboard_ = useBillboard; }
	void SetUseField(bool useField) { this->useField_ = useField; }

	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);

	std::list<Particle> Emit(const Emitter& emitter, std::mt19937& randomEngine);
	void UpdateEmitter();

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

	std::list<Particle> particles_;

	const float kDeltaTime = 1.0f / 60.0f;
	uint32_t numInstance = 0;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

	// ビルボードを使用しているか
	bool useBillboard_ = false;

	// 風を使用してるか
	bool useField_ = false;

	Emitter emitter_;

	// フィールド
	AccelerationField field_ = {
	  { 15.0f, 0.0f, 0.0f },
	  {{-1.0f, -1.0f, -1.0f},
	  { 1.0f,  1.0f,  1.0f }}
	};

};

