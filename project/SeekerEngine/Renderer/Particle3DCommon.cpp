#include "Particle3DCommon.h"

void Particle3DCommon::Init(Graphics* graphics, DxcCompiler& dxcCompiler, ID3D12RootSignature* rootSignature)
{
    graphics_ = graphics;
    rootSignature_ = rootSignature;

    CreateGraphicsPipeline(graphics, dxcCompiler);

	// マテリアルリソースを作る
	materialResource = CreateBufferResource(Graphics::GetDevice(), sizeof(Material));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// SpriteはLightingしないのでfalseを設定する
	materialData->color = Vector4(1, 1, 1, 1);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();

	// 板ポリモデル生成
	CreatePlaneModel();
	// インスタンス用バッファ
	CreateInstanceResource();

    cmdList_ = Graphics::GetCmdList();

	/*for (uint32_t i = 0; i < kNumMaxInstance_; ++i) {
		std::random_device seedGenerator;
		std::mt19937 randomEngine(seedGenerator());
		particle_[i] = MakeNewParticle(randomEngine);
	}*/
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
	cmdList_->DrawIndexedInstanced(6, numInstance, 0, 0, 0);
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
	depthStencilDesc_ = {};
	// DepthStencilStateの設定
	depthStencilDesc_.DepthEnable = true;
	// 書き込みします
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
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
		vsBlob_,
		psBlob_,
		blendDesc_,
		rasterizerDesc,
		depthStencilDesc_
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

   numInstance = 0;  
   for (std::list<Particle>::iterator particleIterator = particles_.begin();
	   particleIterator != particles_.end(); ) {
       if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {  
		   particleIterator = particles_.erase(particleIterator);
           continue;  
       }  

	   if (useField_ && IsCollision(field_.area, (*particleIterator).transform.translate)) {
			 (*particleIterator).velocity.x += field_.acceleration.x * kDeltaTime;
			 (*particleIterator).velocity.y += field_.acceleration.y * kDeltaTime;
			 (*particleIterator).velocity.z += field_.acceleration.z * kDeltaTime;
	   }

	   particleIterator->transform.translate.x += particleIterator->velocity.x * kDeltaTime;
	   particleIterator->transform.translate.y += particleIterator->velocity.y * kDeltaTime;
	   particleIterator->transform.translate.z += particleIterator->velocity.z * kDeltaTime;
	   particleIterator->currentTime += kDeltaTime;
       float alpha = 1.0f - (particleIterator->currentTime / particleIterator->lifeTime);

       Matrix4x4 wvpMatrix;  
       Matrix4x4 worldMatrix;  

       if (useBillboard_) {  
           Matrix4x4 scaleMatrix = MakeScaleMatrix(particleIterator->transform.scale);
           Matrix4x4 translateMatrix = MakeTranslateMatrix(particleIterator->transform.translate);

           if (camera_) {  
			   const Matrix4x4& billBoardMatrix = camera_->GetBillboardMatrix();
               worldMatrix = scaleMatrix * billBoardMatrix * translateMatrix;
           } else {
               worldMatrix = scaleMatrix * translateMatrix;  
           }  

       } else {  
           // World行列  
           worldMatrix = MakeAffineMatrix(  
			   particleIterator->transform.scale,
			   particleIterator->transform.rotate,
			   particleIterator->transform.translate);
       }  

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

	   if (numInstance < kNumMaxInstance_) {
		   // 書き込み  
		   instancingData_[numInstance].World = worldMatrix;
		   instancingData_[numInstance].WVP = wvpMatrix;
		   instancingData_[numInstance].color = particleIterator->color;
		   instancingData_[numInstance].color.w = alpha;
		   ++numInstance;
	   }

	   ++particleIterator;
   }  
}

Particle Particle3DCommon::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate)
{
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);
	Particle particle;
	Vector3 randomTranslate = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };

	// スケール（すべて同じ
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.transform.translate = { translate + randomTranslate };
	particle.color = { distColor(randomEngine), distColor(randomEngine), distColor(randomEngine), 1.0f };
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;

	return particle;
}

std::list<Particle> Particle3DCommon::Emit(const Emitter& emitter, std::mt19937& randomEngine)
{
	emitter_ = emitter;
	std::list<Particle> particles;

	for (uint32_t count = 0; count < emitter.count; ++count) {
		particles.push_back(MakeNewParticle(randomEngine, emitter.transform.translate));
	}

	return particles;
}

void Particle3DCommon::UpdateEmitter()
{
	std::random_device rd;
	std::mt19937 randomEngine(rd());
	emitter_.frequencyTime += kDeltaTime;
	if (emitter_.frequency <= emitter_.frequencyTime) {
		particles_.splice(particles_.end(), Emit(emitter_, randomEngine));
		emitter_.frequencyTime -= emitter_.frequency;
	}
}

void Particle3DCommon::CreateGraphicsPipeline(Graphics* graphics, DxcCompiler& dxcCompiler)
{
	depthStencilDesc_ = {};
	// DepthStencilStateの設定
	depthStencilDesc_.DepthEnable = true;
	// 書き込みします
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
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
		rasterizerDesc,
		depthStencilDesc_
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

	size_t bufferSize = sizeof(ParticleForGPU) * kNumMaxInstance_;
	instancingResource_ = CreateBufferResource(Graphics::GetDevice(), bufferSize);

	instancingData_ = nullptr;
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

	for (uint32_t i = 0; i < kNumMaxInstance_; ++i) {
		instancingData_[i].WVP = MakeIdentity4x4();
		instancingData_[i].World = MakeIdentity4x4();
		instancingData_[i].color = { 1,1,1,1 };
	}

	Logger::Write("Particle instancing buffer created");

	SrvManager::GetInstance()->CreateSRVforStructuredBuffer(
		instanceSrvIndex_,
		instancingResource_.Get(),
		kNumMaxInstance_,
		sizeof(ParticleForGPU)
	);

	Logger::Write("Particle instancing SRV created");
}