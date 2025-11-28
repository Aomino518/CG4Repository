#include "Particle3DCommon.h"

void Particle3DCommon::Init(Graphics* graphics, DxcCompiler& dxcCompiler, ID3D12RootSignature* rootSignature)
{
    graphics_ = graphics;
    rootSignature_ = rootSignature;

    CreateGraphicsPipeline(graphics, dxcCompiler);
	// 板ポリモデル生成
	CreatePlaneModel();
	// インスタンス用バッファ
	CreateInstanceResource();

	// マテリアルリソースを作る
	materialResource = CreateBufferResource(Graphics::GetDevice(), sizeof(Material));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// SpriteはLightingしないのでfalseを設定する
	materialData->color = Vector4(1, 1, 1, 1);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();

    cmdList_ = Graphics::GetCmdList();
}

void Particle3DCommon::DrawCommon()
{
	cmdList_->SetGraphicsRootSignature(rootSignature_.Get());
	cmdList_->SetPipelineState(psoParticle3D_.Get());
	cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Particle3DCommon::Draw()
{
	if (!cmdList_) {
		cmdList_ = Graphics::GetCmdList();
	}

	DrawCommon();

	SrvManager::GetInstance()->PreDraw();

	cmdList_->IASetVertexBuffers(0, 1, &vbView_);
	cmdList_->IASetIndexBuffer(&ibView_);
	cmdList_->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	cmdList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(3, instanceSrvIndex_);
	cmdList_->DrawIndexedInstanced(6, kNumInstance_, 0, 0, 0);
}

void Particle3DCommon::SetBlendMode(BlendMode mode)
{
	if (mode_ == mode)
	{
		return;
	}

	mode_ = mode;

	if (psoCache_.contains(mode)) {
		psoParticle3D_ = psoCache_[mode];
	} else {
		RebuildPso();
		psoCache_[mode] = psoParticle3D_;
	}
}

void Particle3DCommon::RebuildPso()
{
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
		vsBlob_,
		psBlob_,
		blendDesc_,
		rasterizerDesc
	);

	psoParticle3D_ = builder.BuildPso(psoDesc);
}

void Particle3DCommon::UpdateInstanceData(CameraManager* cameraManager)
{
	cameraManager_ = cameraManager;
	debugCamera_ = cameraManager->GetDebugCamera();
	camera_ = cameraManager->GetActiveCamera();

	if (!instancingData_) {
		return;
	}

	bool isDebug = cameraManager_->GetIsDebug();

	for (uint32_t i = 0; i < kNumInstance_; ++i) {

		// パーティクルの座標（例：横方向に 2.0f ずつずらす）
		float x = i * 0.1f;
		float y = i * 0.1f;
		float z = i * 0.1f;

		// スケール（すべて同じ）
		Vector3 scale = { 1.0f, 1.0f, 1.0f };
		Vector3 rotate = { 0.0f, 0.0f, 0.0f };
		Vector3 translate = { x, y, z };

		Matrix4x4 wvpMatrix;
		// World行列
		Matrix4x4 worldMatrix = MakeAffineMatrix(scale, rotate, translate);

		if (isDebug) {
			if (debugCamera_) {
				const Matrix4x4& viewProjectionMatrix = debugCamera_->GetViewProjectionMatrix();
				wvpMatrix = Multiply(worldMatrix, viewProjectionMatrix);
			} else {
				wvpMatrix = worldMatrix;
			}
		} else {
			if (camera_) {
				const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
				wvpMatrix = Multiply(worldMatrix, viewProjectionMatrix);
			} else {
				wvpMatrix = worldMatrix;
			}
		}

		// 書き込み
		instancingData_[i].World = worldMatrix;
		instancingData_[i].WVP = wvpMatrix;
	}
}

void Particle3DCommon::CreateGraphicsPipeline(Graphics* graphics, DxcCompiler& dxcCompiler)
{
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
	vsBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/Particle3D.VS.hlsl", L"vs_6_0");
	psBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/Particle3D.PS.hlsl", L"ps_6_0");

	// PSOを生成する
	// 3D用
	PsoBuilder builder;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc3D{};
	builder.Init(graphics);
	psoDesc3D = builder.CreatePsoDesc(
		rootSignature_,
		inputLayoutDesc3D,
		vsBlob_,
		psBlob_,
		blendDesc_,
		rasterizerDesc
	);

	psoParticle3D_ = builder.BuildPso(psoDesc3D);
	Logger::Write("pipelineState生成完了");
}

void Particle3DCommon::CreatePlaneModel()
{
	VertexData vertices[6]{
		{ { -1.0f,  1.0f, 0.0f, 1.0f }, {0.0f, 0.0f}, {0.0f,0.0f,1.0f} },
		{ {  1.0f,  1.0f, 0.0f, 1.0f }, {1.0f, 0.0f}, {0.0f,0.0f,1.0f} },
		{ { -1.0f, -1.0f, 0.0f, 1.0f }, {0.0f, 1.0f}, {0.0f,0.0f,1.0f} },
		{ {  1.0f,  1.0f, 0.0f, 1.0f }, {1.0f, 0.0f}, {0.0f,0.0f,1.0f} },
		{ {  1.0f, -1.0f, 0.0f, 1.0f }, {1.0f, 1.0f}, {0.0f,0.0f,1.0f} },
		{ { -1.0f, -1.0f, 0.0f, 1.0f }, {0.0f, 1.0f}, {0.0f,0.0f,1.0f} }
	};

	uint32_t indices[6] = { 0,1,2,3,4,5 };

	size_t vertexBufferSize = sizeof(vertices);

	vertexBuffer_ = CreateBufferResource(Graphics::GetDevice(), vertexBufferSize);

	// 書き込み
	VertexData* vbPtr = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vbPtr));
	memcpy(vbPtr, vertices, vertexBufferSize);
	vertexBuffer_->Unmap(0, nullptr);

	vbView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vbView_.SizeInBytes = (UINT)vertexBufferSize;
	vbView_.StrideInBytes = sizeof(VertexData);

	// インデックスバッファ生成
	size_t indexBufferSize = sizeof(indices);

	indexBuffer_ = CreateBufferResource(Graphics::GetDevice(), indexBufferSize);

	uint32_t* ibPtr = nullptr;
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&ibPtr));
	memcpy(ibPtr, indices, indexBufferSize);
	indexBuffer_->Unmap(0, nullptr);

	ibView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	ibView_.SizeInBytes = (UINT)indexBufferSize;
	ibView_.Format = DXGI_FORMAT_R32_UINT;

	Logger::Write("Particle Plane Model Generated");
}

void Particle3DCommon::CreateInstanceResource()
{
	instanceSrvIndex_ = SrvManager::GetInstance()->Allocate();

	size_t bufferSize = sizeof(TransformationMatrix) * kNumInstance_;
	instancingResource_ = CreateBufferResource(Graphics::GetDevice(), bufferSize);

	instancingData_ = nullptr;
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

	for (uint32_t i = 0; i < kNumInstance_; ++i) {
		instancingData_[i].WVP = MakeIdentity4x4();
		instancingData_[i].World = MakeIdentity4x4();
	}

	instancingResource_->Unmap(0, nullptr);

	Logger::Write("Particle instancing buffer created");

	SrvManager::GetInstance()->CreateSRVforStructuredBuffer(
		instanceSrvIndex_,
		instancingResource_.Get(),
		kNumInstance_,
		sizeof(TransformationMatrix)
	);

	Logger::Write("Particle instancing SRV created");
}
