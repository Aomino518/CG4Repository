#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <Graphics.h>
#include <DxcCompiler.h>
#include <InputLayout.h>
#include "PsoBuilder.h"

class Entity3DCommon
{
public:
	void Init(DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature);

	void DrawCommon();

private:
	// グラフィックパイプラインの作成
	void CreateGraphicPipeline(Graphics* graphics, DxcCompiler dxcCompiler);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Graphics* graphics_;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso3D_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;
};

