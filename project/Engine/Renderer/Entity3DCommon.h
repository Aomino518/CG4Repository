#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <Graphics.h>
#include <DxcCompiler.h>
#include <InputLayout.h>
#include "PsoBuilder.h"
#include "Camera.h"
#include "DebugCamera.h"
#include "CameraManager.h"

class Entity3DCommon
{
public:
	void Init(Graphics* graphics, DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature);

	void DrawCommon();

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCmdList() const { return cmdList_; }

	// Getter
	Camera* GetDefaultCamera() const { return defaultCamera_; }
	DebugCamera* GetDebugCamera() const { return debugCamera_; }
	CameraManager* GetCameraManager() const { return cameraManager_; }
	// Setter
	void SetDefaultCamera(Camera* camera) { this->defaultCamera_ = camera; }
	void SetCameraManager(CameraManager* cameraManager) { this->cameraManager_ = cameraManager; }
	void SetDebugCamera(DebugCamera* debugCamera) { this->debugCamera_ = debugCamera; }

private:
	// グラフィックパイプラインの作成
	void CreateGraphicPipeline(Graphics* graphics, DxcCompiler dxcCompiler);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Graphics* graphics_;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso3D_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;

	Camera* defaultCamera_ = nullptr;
	DebugCamera* debugCamera_ = nullptr;
	CameraManager* cameraManager_ = nullptr;
};

