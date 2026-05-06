#include "Skybox.h"
#include "Graphics.h"
#include "Logger.h"

Skybox* Skybox::GetInstance()
{
	static Skybox instance;
	return &instance;
}

void Skybox::Init(DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature)
{
	graphics_ = Graphics::GetInstance();
	rootSignature_ = rootSignature;
	this->cameraManager_ = CameraManager::GetInstance();
	CreateGraphicPipeline(dxcCompiler);
	cmdList_ = Graphics::GetCmdList();
	CreateVertexBuffer();
	CreateTransformationMatrixResource();
}

void Skybox::Update()
{
	Matrix4x4 worldMatrix = MakeIdentity4x4();
	// WVPMatrixを作る
	Matrix4x4 viewProjectionMatrix = worldMatrix;

	if (cameraManager_->GetIsDebug()) {
		auto* debugCamera = cameraManager_->GetDebugCamera();
		if (debugCamera) {
			Matrix4x4 viewMatrix = debugCamera->GetViewMatrix();

			viewMatrix.m[3][0] = 0.0f;
			viewMatrix.m[3][1] = 0.0f;
			viewMatrix.m[3][2] = 0.0f;

			viewProjectionMatrix = Multiply(viewMatrix, debugCamera->GetProjectionMatrix());
		}
	} else {
		auto* camera = cameraManager_->GetActiveCamera();
		if (camera) {
			Matrix4x4 viewMatrix = camera->GetViewMatrix();

			viewMatrix.m[3][0] = 0.0f;
			viewMatrix.m[3][1] = 0.0f;
			viewMatrix.m[3][2] = 0.0f;

			viewProjectionMatrix = Multiply(viewMatrix, camera->GetProjectionMatrix());
		}
	}

	transformationMatrixData_->World = worldMatrix;
	transformationMatrixData_->WVP = viewProjectionMatrix;
}

void Skybox::Draw()
{
	cmdList_->SetGraphicsRootSignature(rootSignature_.Get());
	cmdList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	cmdList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	cmdList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	cmdList_->SetPipelineState(pso_.Get());
	cmdList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	cmdList_->IASetIndexBuffer(&indexBufferView_);
	cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList_->DrawIndexedInstanced(36, 1, 0, 0, 0);
	Graphics::GetInstance()->AddDrawCallCount();
}

void Skybox::Shutdown()
{
	pso_.Reset();
	vsBlob_.Reset();
	psBlob_.Reset();
	rootSignature_.Reset();
	graphics_ = nullptr;
	cmdList_.Reset();
	Logger::Write("Skybox Shutdown");
}

void Skybox::SetTexture(uint32_t textureId)
{
	textureIndex_ = textureId;
	textureSrvHandleGPU_ = TextureManager::GetInstance()->GetGPUHandle(textureId);
}

void Skybox::CreateGraphicPipeline(DxcCompiler dxcCompiler)
{
	depthStencilDesc_ = {};
	// DepthStencilStateの設定
	depthStencilDesc_.DepthEnable = true;
	// 書き込みします
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	//depthStencilDesc_.StencilEnable = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDescDebug3D{};
	InputLayout inputLayout;
	inputLayoutDescDebug3D = inputLayout.CreateInputLayout3D();

	// BlendStateの設定
	// すべての色要素を書き込む
	blendDesc_ = CreateBlendDesc(mode_);
	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;

	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	vsBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/Skybox.VS.hlsl", L"vs_6_0");
	psBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/Skybox.PS.hlsl", L"ps_6_0");

	// PSOを生成する
	PsoBuilder builder;
	builder.Init(graphics_);
	// Wire用
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc = builder.CreatePsoDesc(
		rootSignature_,
		inputLayoutDescDebug3D,
		vsBlob_,
		psBlob_,
		blendDesc_,
		rasterizerDesc,
		depthStencilDesc_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	pso_ = builder.BuildPso(psoDesc);
	Logger::Write("PSO Skybox生成完了");
}

void Skybox::CreateVertexBuffer()
{
	vertexResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(VertexData) * 24);
	indexResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(uint32_t) * 36);

	// VertexBufferViewを作成する
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 24;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	Logger::Write("VertexBufferView生成完了");

	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	// 前面
	vertexData_[0].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData_[1].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData_[2].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertexData_[3].position = { 1.0f, -1.0f, -1.0f, 1.0f };

	// 後面
	vertexData_[4].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertexData_[5].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData_[6].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	vertexData_[7].position = { -1.0f, -1.0f, 1.0f, 1.0f };

	// 右面
	vertexData_[8].position = vertexData_[1].position;
	vertexData_[9].position = vertexData_[3].position;
	vertexData_[10].position = vertexData_[4].position;
	vertexData_[11].position = vertexData_[6].position;

	// 左面
	vertexData_[12].position = vertexData_[5].position;
	vertexData_[13].position = vertexData_[7].position;
	vertexData_[14].position = vertexData_[0].position;
	vertexData_[15].position = vertexData_[2].position;

	// 上面
	vertexData_[16].position = vertexData_[0].position;
	vertexData_[17].position = vertexData_[1].position;
	vertexData_[18].position = vertexData_[5].position;
	vertexData_[19].position = vertexData_[4].position;

	// 下面
	vertexData_[20].position = vertexData_[7].position;
	vertexData_[21].position = vertexData_[6].position;
	vertexData_[22].position = vertexData_[2].position;
	vertexData_[23].position = vertexData_[3].position;

	// IndexBufferViewを作成する
	// リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 36;
	// インデックスはuint32_tとする
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	Logger::Write("IndexBufferView生成完了");

	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	// 前面
	indexData_[0] = 0;
	indexData_[1] = 1;
	indexData_[2] = 2;
	indexData_[3] = 2;
	indexData_[4] = 1;
	indexData_[5] = 3;

	// 後面
	indexData_[6] = 4;
	indexData_[7] = 5;
	indexData_[8] = 6;
	indexData_[9] = 6;
	indexData_[10] = 5;
	indexData_[11] = 7;

	// 右面
	indexData_[12] = 8;
	indexData_[13] = 10;
	indexData_[14] = 9;
	indexData_[15] = 9;
	indexData_[16] = 10;
	indexData_[17] = 11;

	// 左面
	indexData_[18] = 12;
	indexData_[19] = 14;
	indexData_[20] = 13;
	indexData_[21] = 13;
	indexData_[22] = 14;
	indexData_[23] = 15;

	// 上面
	indexData_[24] = 16;
	indexData_[25] = 18;
	indexData_[26] = 17;
	indexData_[27] = 17;
	indexData_[28] = 18;
	indexData_[29] = 19;

	// 下面
	indexData_[30] = 20;
	indexData_[31] = 22;
	indexData_[32] = 21;
	indexData_[33] = 21;
	indexData_[34] = 22;
	indexData_[35] = 23;

	Logger::Write("indexDataに割り当て完了");

	// マテリアルリソースを作る
	materialResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// SpriteはLightingしないのでfalseを設定する
	materialData_->color = Vector4(1, 1, 1, 1);
	materialData_->enableLighting = false;
	materialData_->uvTransform = MakeIdentity4x4();
}

void Skybox::CreateTransformationMatrixResource()
{
	// TransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	// 単位行列を書きこんでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();

	// カメラリソース
	cameraResource = CreateBufferResource(Graphics::GetDevice(), sizeof(CameraForGPU));
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));

	Vector3 camPos = { 0.0f, 0.0f, 0.0f };
	if (cameraManager_->GetIsDebug()) {
		if (debugCamera_) {
			camPos = debugCamera_->GetTranslate();
		}
	} else if (camera_) {
		camPos = camera_->GetTranslate();
	}
	cameraData_->worldPosition = camPos;
}
