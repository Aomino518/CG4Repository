#include "Entity3DCommon.h"

void Entity3DCommon::Init(Graphics* graphics, DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature)
{
	graphics_ = graphics;
	rootSignature_ = rootSignature;
	CreateGraphicPipeline(graphics, dxcCompiler);
	cmdList_ = Graphics::GetCmdList();
}

void Entity3DCommon::DrawCommon()
{
	cmdList_->SetGraphicsRootSignature(rootSignature_.Get());
	cmdList_->SetPipelineState(pso3D_.Get());
	cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Entity3DCommon::SetBlendMode(BlendMode mode)
{
	if (mode_ == mode)
	{
		return;
	}

	mode_ = mode;

	if (psoCache_.contains(mode)) {
		pso3D_ = psoCache_[mode];
	} else {
		RebuildPso();
		psoCache_[mode] = pso3D_;
	}
}

void Entity3DCommon::CreateGraphicPipeline(Graphics* graphics, DxcCompiler dxcCompiler)
{
	depthStencilDesc_ = {};
	// DepthStencilStateの設定
	// Depthの機能を有効化する
	depthStencilDesc_.DepthEnable = true;
	// 書き込みします
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// InputLayout
	InputLayout inputLayout;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc3D{};
	inputLayoutDesc3D = inputLayout.CreateInputLayout3D();

	blendDesc_ = CreateBlendDesc(mode_);

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	vs3DBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/Object3D.VS.hlsl", L"vs_6_0");
	ps3DBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/Object3D.PS.hlsl", L"ps_6_0");

	// PSOを生成する
	// 3D用
	PsoBuilder builder;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc3D{};
	builder.Init(graphics);
	psoDesc3D = builder.CreatePsoDesc(
		rootSignature_,
		inputLayoutDesc3D,
		vs3DBlob_,
		ps3DBlob_,
		blendDesc_,
		rasterizerDesc,
		depthStencilDesc_
	);

	pso3D_ = builder.BuildPso(psoDesc3D);
	Logger::Write("PSO3D生成完了");
}

void Entity3DCommon::RebuildPso()
{
	depthStencilDesc_ = {};
	// DepthStencilStateの設定
	// Depthの機能を有効化する
	depthStencilDesc_.DepthEnable = true;
	// 書き込みします
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	InputLayout inputLayout;
	D3D12_INPUT_LAYOUT_DESC layout = inputLayout.CreateInputLayout3D();

	blendDesc_ = CreateBlendDesc(mode_);

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	PsoBuilder builder;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	builder.Init(graphics_);
	psoDesc = builder.CreatePsoDesc(
		rootSignature_.Get(),
		layout,
		vs3DBlob_,
		ps3DBlob_,
		blendDesc_,
		rasterizerDesc,
		depthStencilDesc_
	);

	pso3D_ = builder.BuildPso(psoDesc);
}