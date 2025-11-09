#include "Entity3DCommon.h"

void Entity3DCommon::Init(Graphics* graphics, DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature)
{
	rootSignature_ = rootSignature;
	CreateGraphicPipeline(graphics, dxcCompiler);
	cmdList_ = graphics->GetCmdList();
}

void Entity3DCommon::DrawCommon()
{
	cmdList_->SetGraphicsRootSignature(rootSignature_.Get());
	cmdList_->SetPipelineState(pso3D_.Get());
	cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Entity3DCommon::CreateGraphicPipeline(Graphics* graphics, DxcCompiler dxcCompiler)
{
	// InputLayout
	InputLayout inputLayout;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc3D{};
	inputLayoutDesc3D = inputLayout.CreateInputLayout3D();

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vs3DBlob = dxcCompiler.CompileShader(L"resources/hlsl/Object3D.VS.hlsl", L"vs_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> ps3DBlob = dxcCompiler.CompileShader(L"resources/hlsl/Object3D.PS.hlsl", L"ps_6_0");

	// PSOを生成する
	// 3D用
	PsoBuilder builder;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc3D{};
	builder.Init(graphics);
	psoDesc3D = builder.CreatePsoDesc(
		rootSignature_,
		inputLayoutDesc3D,
		vs3DBlob,
		ps3DBlob,
		blendDesc,
		rasterizerDesc
	);

	pso3D_ = builder.BuildPso(psoDesc3D);
	Logger::Write("PSO3D生成完了");
}
