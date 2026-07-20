#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <DxcCompiler.h>
#include <InputLayout.h>
#include "PsoBuilder.h"
#include "Camera.h"
#include "DebugCamera.h"
#include "CameraManager.h"
#include "BlendStateUtils.h"

class Graphics;
class Entity3DCommon
{
public:
	// シングルトンインスタンスの取得
	static Entity3DCommon* GetInstance();

	void Init(DxcCompiler dxcCompiler, ID3D12RootSignature* normalRootSignature,
		ID3D12RootSignature* skinningRootSignature);

	void ApplyPipeline(ModelRenderType renderType);

	void Shutdown();

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCmdList() const { return cmdList_; }

	// Getter
	Camera* GetDefaultCamera() const { return defaultCamera_; }
	DebugCamera* GetDebugCamera() const { return debugCamera_; }
	CameraManager* GetCameraManager() const { return cameraManager_; }
	BlendMode& GetBlendMode() { return mode_; }
	// Setter
	void SetDefaultCamera(Camera* camera) { this->defaultCamera_ = camera; }
	void SetCameraManager(CameraManager* cameraManager) { this->cameraManager_ = cameraManager; }
	void SetDebugCamera(DebugCamera* debugCamera) { this->debugCamera_ = debugCamera; }
	void SetBlendMode(BlendMode mode, ModelRenderType type);

private:
	Entity3DCommon() = default;
	~Entity3DCommon() = default;
	Entity3DCommon(const Entity3DCommon&) = delete;
	Entity3DCommon& operator=(const Entity3DCommon&) = delete;

	// グラフィックパイプラインの作成
	void CreateGraphicPipeline(DxcCompiler dxcCompiler);
	void CreateNormalPso();
	void CreateSkinningPso();

	void RebuildPso(ModelRenderType type);

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> normalRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> skinningRootSignature_;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Graphics* graphics_;

	Microsoft::WRL::ComPtr<IDxcBlob> normalVsBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> skinningVsBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> objectPsBlob_;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> normalPso_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> skinningPso_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;

	Camera* defaultCamera_ = nullptr;
	DebugCamera* debugCamera_ = nullptr;
	CameraManager* cameraManager_ = nullptr;

	D3D12_BLEND_DESC blendDesc_{};
	BlendMode mode_ = kBlendModeNormal;
	std::unordered_map<BlendMode, Microsoft::WRL::ComPtr<ID3D12PipelineState>> normalPsoCache_;
	std::unordered_map<BlendMode, Microsoft::WRL::ComPtr<ID3D12PipelineState>> skinningPsoCache_;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};
};