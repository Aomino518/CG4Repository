#include "Entity3DCommon.h"
#include <Graphics.h>
#include "Logger.h"

Entity3DCommon* Entity3DCommon::GetInstance()
{
	static Entity3DCommon instance;
	return &instance;
}

void Entity3DCommon::Init(DxcCompiler dxcCompiler, ID3D12RootSignature* normalRootSignature,
	ID3D12RootSignature* skinningRootSignature)
{
	graphics_ = Graphics::GetInstance();
	normalRootSignature_ = normalRootSignature;
	skinningRootSignature_ = skinningRootSignature;
	CreateGraphicPipeline(dxcCompiler);
	cmdList_ = Graphics::GetInstance()->GetCmdList();
}

void Entity3DCommon::ApplyPipeline(ModelRenderType renderType)
{
	// Skinningか通常のモデルか判定
	if (renderType == ModelRenderType::Normal) {
		cmdList_->SetGraphicsRootSignature(normalRootSignature_.Get());
		cmdList_->SetPipelineState(normalPso_.Get());
	} else if (renderType == ModelRenderType::Skinning) {
		cmdList_->SetGraphicsRootSignature(skinningRootSignature_.Get());
		cmdList_->SetPipelineState(skinningPso_.Get());
	}
	cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Entity3DCommon::Shutdown()
{
	normalRootSignature_.Reset();
	skinningRootSignature_.Reset();
	pipelineState_.Reset();
	normalVsBlob_.Reset();
	skinningVsBlob_.Reset();
	objectPsBlob_.Reset();
	normalPso_.Reset();
	skinningPso_.Reset();
	cmdList_.Reset();
	Logger::Write("Entity3DCommon Shutdown");
}

void Entity3DCommon::SetBlendMode(BlendMode mode, ModelRenderType type)
{
	if (mode_ == mode)
	{
		return;
	}

	mode_ = mode;

	if (type == ModelRenderType::Normal) {
		if (normalPsoCache_.contains(mode)) {
			normalPso_ = normalPsoCache_[mode];
		} else {
			RebuildPso(type);
			normalPsoCache_[mode] = normalPso_;
		}
	} else if (type == ModelRenderType::Skinning) {
		if (skinningPsoCache_.contains(mode)) {
			skinningPso_ = skinningPsoCache_[mode];
		} else {
			RebuildPso(type);
			skinningPsoCache_[mode] = skinningPso_;
		}
	}
}

void Entity3DCommon::CreateGraphicPipeline(DxcCompiler dxcCompiler)
{
	normalVsBlob_ = dxcCompiler.CompileShader(
		L"resources/hlsl/Object3D.VS.hlsl",
		L"vs_6_0"
	);

	skinningVsBlob_ = dxcCompiler.CompileShader(
		L"resources/hlsl/SkinningObject3d.VS.hlsl",
		L"vs_6_0"
	);

	objectPsBlob_ = dxcCompiler.CompileShader(
		L"resources/hlsl/Object3D.PS.hlsl",
		L"ps_6_0"
	);

	CreateNormalPso();
	CreateSkinningPso();
}

void Entity3DCommon::CreateNormalPso()
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

	// PSOを生成する
	// 3D用
	PsoBuilder builder;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	builder.Init(graphics_);
	psoDesc = builder.CreatePsoDesc(
		normalRootSignature_,
		inputLayoutDesc3D,
		normalVsBlob_,
		objectPsBlob_,
		blendDesc_,
		rasterizerDesc,
		depthStencilDesc_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	normalPso_ = builder.BuildPso(psoDesc);
	Logger::Write("NormalPso生成完了");
}

void Entity3DCommon::CreateSkinningPso()
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
	inputLayoutDesc3D = inputLayout.CreateInputLayout3dSkinning();

	blendDesc_ = CreateBlendDesc(mode_);

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// PSOを生成する
	// 3D用
	PsoBuilder builder;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	builder.Init(graphics_);
	psoDesc = builder.CreatePsoDesc(
		skinningRootSignature_,
		inputLayoutDesc3D,
		skinningVsBlob_,
		objectPsBlob_,
		blendDesc_,
		rasterizerDesc,
		depthStencilDesc_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	skinningPso_ = builder.BuildPso(psoDesc);
	Logger::Write("SkinningPso生成完了");
}

void Entity3DCommon::RebuildPso(ModelRenderType type)
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
	if (type == ModelRenderType::Skinning) {
		layout = inputLayout.CreateInputLayout3dSkinning();
	}

	blendDesc_ = CreateBlendDesc(mode_);

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	PsoBuilder builder;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	builder.Init(graphics_);

	if (type == ModelRenderType::Normal) {
		psoDesc = builder.CreatePsoDesc(
			normalRootSignature_.Get(),
			layout,
			normalVsBlob_,
			objectPsBlob_,
			blendDesc_,
			rasterizerDesc,
			depthStencilDesc_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
		);

		normalPso_ = builder.BuildPso(psoDesc);
	} else if (type == ModelRenderType::Skinning) {
		psoDesc = builder.CreatePsoDesc(
			skinningRootSignature_.Get(),
			layout,
			skinningVsBlob_,
			objectPsBlob_,
			blendDesc_,
			rasterizerDesc,
			depthStencilDesc_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
		);

		skinningPso_ = builder.BuildPso(psoDesc);
	}
}