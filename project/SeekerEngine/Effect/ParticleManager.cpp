#include "ParticleManager.h"
#include "Graphics.h"
#include "EmitterManager.h"
#include "WorldFieldManager.h"

ParticleManager* ParticleManager::GetInstance()
{
	static ParticleManager instance;
	return &instance;
}

void ParticleManager::Init(DxcCompiler& dxcCompiler, ID3D12RootSignature* rootSignature)
{
	graphics_ = Graphics::GetInstance();
	rootSignature_ = rootSignature;
	// ランダムエンジンの初期化
	std::random_device seed;
	randomEngine_ = std::mt19937(seed());

	// グラフィックパイプラインを生成
	CreateGraphicsPipeline(dxcCompiler);
	// 板ポリの生成
	CreatePlaneModel();
	CreateRingModel();
	CreateCylinderModel();

	cmdList_ = Graphics::GetCmdList();
}

void ParticleManager::Update(CameraManager* cameraManager)
{
	cameraManager_ = cameraManager;
	debugCamera_ = cameraManager->GetDebugCamera();
	camera_ = cameraManager->GetActiveCamera();

	bool isDebug = cameraManager_->GetIsDebug();

	for (auto& [name, group] : particleGroups) {
		group.instanceCount = 0;

		// UVスクロール
		group.uvOffset.x += group.uvScrollSpeed.x * kDeltaTime;
		group.uvOffset.y += group.uvScrollSpeed.y * kDeltaTime;
		group.uvOffset.x = std::fmod(group.uvOffset.x, 1.0f);
		group.uvOffset.y = std::fmod(group.uvOffset.y, 1.0f);

		Matrix4x4 uvTranslation = MakeTranslateMatrix({ group.uvOffset.x, group.uvOffset.y, 0.0f });
		group.materialData->uvTransform = uvTranslation;
		group.materialData->alphaReference = group.alphaReference;

		for (auto particleIterator = group.particles.begin(); particleIterator != group.particles.end(); ) {
			// 移動と時間の更新
			if (particleIterator->lifeTime <= particleIterator->currentTime) {
				particleIterator = group.particles.erase(particleIterator);
				continue;
			}

			auto emitters = EmitterManager::GetInstance()->GetAllEmitters();
			auto worldFields = WorldFieldManager::GetInstance()->GetAllWorldFields();
			// ワールドフィールドの更新
			Vector3 totalAcceleration{ 0.0f, 0.0f, 0.0f };
			for (const auto& field : worldFields) {
				if (!field->GetIsActive()) {
					continue;
				}

				if (IsCollision(field->GetWorldAABB(), particleIterator->transform.translate)) {
					totalAcceleration = totalAcceleration + field->GetAcceleration();
				}
			}

			// ローカルフィールドの更新
			for (const auto& emitter : emitters) {
				Transform transform = emitter->GetTransform();
				auto& field = emitter->GetLocalField();
				if (!field.GetIsActive()) {
					continue;
				}

				if (IsCollision(field.GetWorldAABB(transform.translate), particleIterator->transform.translate)) {
					totalAcceleration = totalAcceleration + field.GetAcceleration();
				}
			}
			particleIterator->velocity = particleIterator->velocity + totalAcceleration * kDeltaTime;

			particleIterator->transform.translate = particleIterator->transform.translate + particleIterator->velocity * kDeltaTime;
			particleIterator->transform.rotate = particleIterator->transform.rotate + particleIterator->rotateVelocity * kDeltaTime;
			particleIterator->currentTime += kDeltaTime;

			// 色の補間
			float t = particleIterator->currentTime / particleIterator->lifeTime;
			t = std::clamp(t, 0.0f, 1.0f);
			Vector4 color = particleIterator->startColor * (1.0f - t) + particleIterator->endColor * t;
			particleIterator->color = color;

			// スケールの補間
			if (particleIterator->isKeepScale) {
				particleIterator->transform.scale = particleIterator->startScale;
			} else {
				Vector3 scale = particleIterator->startScale * (1.0f - t) + particleIterator->endScale * t;
				particleIterator->transform.scale = scale;
			}

			Matrix4x4 worldMatrix = CalculateWorldMatrix(*particleIterator, name, isDebug);
			Matrix4x4 wvpMatrix = CalculateWVPMatrix(worldMatrix, isDebug);

			if (group.instanceCount < kNumMaxInstance_) {
				// 書き込み  
				group.instanceData[group.instanceCount].World = worldMatrix;
				group.instanceData[group.instanceCount].WVP = wvpMatrix;
				group.instanceData[group.instanceCount].color = color;
				++group.instanceCount;
			}

			++particleIterator;
		}
	}
}

void ParticleManager::Draw()
{
	for (auto& [name, group] : particleGroups) {
		cmdList_->SetGraphicsRootSignature(rootSignature_.Get());
		cmdList_->SetPipelineState(GetPso(group.blendMode_));
		cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		SrvManager::GetInstance()->PreDraw();

		UINT indexCount = 6;
		if (group.modelType_ == ParticleModelType::Plane) {
			cmdList_->IASetVertexBuffers(0, 1, &vbViewPlane_);
			cmdList_->IASetIndexBuffer(&ibViewPlane_);
			indexCount = 6;
		} else if(group.modelType_ == ParticleModelType::Ring) {
			cmdList_->IASetVertexBuffers(0, 1, &vbViewRing_);
			cmdList_->IASetIndexBuffer(&ibViewRing_);
			indexCount = 32 * 6;
		} else if (group.modelType_ == ParticleModelType::Cylinder) {
			cmdList_->IASetVertexBuffers(0, 1, &vbViewCylinder_);
			cmdList_->IASetIndexBuffer(&ibViewCylinder_);
			indexCount = 32 * 6;
		}

		cmdList_->SetGraphicsRootConstantBufferView(0, group.materialResource->GetGPUVirtualAddress());

		if (group.instanceCount == 0) {
			continue;
		}

		cmdList_->SetGraphicsRootDescriptorTable(2, group.textureSrvHandle);
		SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(3, group.srvIndexCount);
		cmdList_->DrawIndexedInstanced(indexCount, group.instanceCount, 0, 0, 0);
	}
}

void ParticleManager::Shutdown()
{
	for (auto& [name, group] : particleGroups) {
		SrvManager::GetInstance()->Free(group.srvIndexCount);

		group.instanceResource.Reset();
	}
	particleGroups.clear();
	rootSignature_.Reset();
	psoParticle3D_.Reset();
	cmdList_.Reset();
	vsBlob_.Reset();
	psBlob_.Reset();
	psoCache_.clear();
	vertexBufferPlane_.Reset();
	indexBufferPlane_.Reset();
	vertexBufferRing_.Reset();
	indexBufferRing_.Reset();
	vertexBufferCylinder_.Reset();
	indexBufferCylinder_.Reset();
	Logger::Write("ParticleManager Shutdown");
}

void ParticleManager::Emit(const std::string name,
	const ParticleConfig& config,
	const Vector3& position,
	uint32_t count)
{
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end());

	ParticleGroup& group = it->second;

	for (uint32_t i = 0; i < count; ++i) {
		Particle particle{};
		Vector3 spawnOffset{};

		// 生成範囲は箱か球か
		if (config.shape == SpawnShape::Box) {
			spawnOffset = RandomRange(randomEngine_, config.boxMin, config.boxMax);
		} else if (config.shape == SpawnShape::Sphere) {
			float theta = RandomRange(randomEngine_, 0.0f, 2.0f * std::numbers::pi_v<float>);
			float phi = RandomRange(randomEngine_, 0.0f, std::numbers::pi_v<float>);
			float r = RandomRange(randomEngine_, 0.0f, config.sphereRadius);

			spawnOffset.x = r * sinf(phi) * cosf(theta);
			spawnOffset.y = r * sinf(phi) * sinf(theta);
			spawnOffset.z = r * cosf(phi);
		}

		particle.transform.translate = { position + spawnOffset };

		// 回転角は固定かランダムか
		if (config.isKeepRotate) {
			particle.transform.rotate = config.rotate;
		} else {
			particle.transform.rotate = RandomRange(randomEngine_, config.minRotate, config.maxRotate);
		}

		// 回転速度が固定かランダムか
		if (config.isKeepRotateVelocity) {
			particle.rotateVelocity = config.rotateVelocity;
		} else {
			particle.rotateVelocity = RandomRange(randomEngine_, config.minRotateVelocity, config.maxRotateVelocity);
		}

		// 速度が固定かランダムか
		if (config.isKeepVelocity) {
			particle.velocity = config.velocity;
		} else {
			particle.velocity = RandomRange(randomEngine_, config.minVelocity, config.maxVelocity);
		}

		// 初期色を固定かランダムか
		if (config.isKeepStartColor) {
			particle.startColor = config.startColor;
		} else {
			particle.startColor = RandomRange(randomEngine_, config.startColorMin, config.startColorMax);
		}

		// 終了色を固定かランダムか
		if (config.isKeepEndColor) {
			particle.endColor = config.endColor;
		} else {
			particle.endColor = RandomRange(randomEngine_, config.endColorMin, config.endColorMax);
		}

		particle.color = particle.startColor;

		// 初期スケールを固定かランダムか
		if (config.isKeepStartScale) {
			particle.startScale = config.startScale;
		} else {
			particle.startScale = RandomRange(randomEngine_, config.startScaleMin, config.startScaleMax);;
		}

		particle.transform.scale = particle.startScale;

		// 終了スケールを固定かランダムか
		if (config.isKeepEndScale) {
			particle.endScale = config.endScale;
		} else {
			particle.endScale = RandomRange(randomEngine_, config.endScaleMin, config.endScaleMax);
		}

		// 生存時間を固定かランダムか
		if (config.isKeepLifeTime) {
			particle.lifeTime = config.lifeTime;
		} else {
			particle.lifeTime = RandomRange(randomEngine_, config.minLifeTime, config.maxLifeTime);
		}

		particle.currentTime = 0.0f;
		particle.isKeepScale = config.isKeepScale;

		group.particles.push_back(particle);
	}
}

void ParticleManager::SetBlendMode(const std::string& name, BlendMode mode)
{
	auto it = particleGroups.find(name);
	if (it == particleGroups.end()) {
		Logger::Write(Logger::LogLevel::Warning, name + " is not Particle");
		return;
	}

	it->second.blendMode_ = mode;
}

void ParticleManager::SetModelType(const std::string& name, ParticleModelType modelType)
{
	auto it = particleGroups.find(name);
	if (it == particleGroups.end()) {
		Logger::Write(Logger::LogLevel::Warning, name + " is not Particle");
		return;
	}

	it->second.modelType_ = modelType;
}

void ParticleManager::RebuildPso()
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
		depthStencilDesc_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	psoParticle3D_ = builder.BuildPso(psoDesc);
}

// パーティクルグループの生成
void ParticleManager::CreateParticleGroup(const std::string& name, const uint32_t textureId)
{
	// 同名がないかチェック
	if (particleGroups.find(name) != particleGroups.end()) {
		Logger::Write(Logger::LogLevel::Warning, "Particle group already exists: " + name);
		return;
	}

	ParticleGroup group{};
	group.textureIndex = textureId;
	group.textureSrvHandle = TextureManager::GetInstance()->GetGPUHandle(textureId);
	group.particles.clear();
	group.instanceCount = 0;
	group.srvIndexCount = SrvManager::GetInstance()->Allocate();
	size_t bufferSize = sizeof(ParticleForGPU) * kNumMaxInstance_;
	group.instanceResource = CreateBufferResource(Graphics::GetDevice(), bufferSize);
	group.instanceData = nullptr;
	group.instanceResource->Map(0, nullptr, reinterpret_cast<void**>(&group.instanceData));
	// マテリアルリソースを作る
	group.materialResource = CreateBufferResource(Graphics::GetDevice(), sizeof(Material));
	group.materialResource->Map(0, nullptr, reinterpret_cast<void**>(&group.materialData));
	// SpriteはLightingしないのでfalseを設定する
	group.materialData->color = Vector4(1, 1, 1, 1);
	group.materialData->enableLighting = false;
	group.materialData->uvTransform = MakeIdentity4x4();
	group.materialData->alphaReference = 0.0f;


	for (uint32_t i = 0; i < kNumMaxInstance_; ++i) {
		group.instanceData[i].WVP = MakeIdentity4x4();
		group.instanceData[i].World = MakeIdentity4x4();
		group.instanceData[i].color = { 1,1,1,1 };
	}

	Logger::Write("Particle instancing buffer created");

	SrvManager::GetInstance()->CreateSRVforStructuredBuffer(
		group.srvIndexCount,
		group.instanceResource.Get(),
		kNumMaxInstance_,
		sizeof(ParticleForGPU)
	);

	Logger::Write("Particle instancing SRV created");

	particleGroups.emplace(name, std::move(group));
}

void ParticleManager::RemoveParticleGroup(const std::string& name)
{
	auto it = particleGroups.find(name);
	if (it == particleGroups.end()) {
		return;
	}

	// SRVとリソースを解放
	SrvManager::GetInstance()->Free(it->second.srvIndexCount);
	it->second.instanceResource.Reset();
	// particlesのコンテナは自動的に破棄
	particleGroups.erase(it);

	Logger::Write("Particle group removed: " + name);
}

ParticleGroup& ParticleManager::GetGroup(const std::string& name)
{
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end() && "Particle group not found");
	return it->second;
}

BlendMode ParticleManager::GetBlendMode(const std::string& name)
{
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end());
	return it->second.blendMode_;
}

ID3D12PipelineState* ParticleManager::GetPso(BlendMode mode)
{
	auto it = psoCache_.find(mode);
	if (it != psoCache_.end()) {
		return it->second.Get();
	}

	mode_ = mode;
	RebuildPso();

	psoCache_[mode] = psoParticle3D_;

	return psoParticle3D_.Get();;
}

// ワールド行列計算関数
Matrix4x4 ParticleManager::CalculateWorldMatrix(const Particle& particle, const std::string& name, bool isDebug)
{
	Matrix4x4 scaleMatrix = MakeScaleMatrix(particle.transform.scale);
	Matrix4x4 rotateMatrix = MakeRotateXMatrix(particle.transform.rotate.x) *
		MakeRotateYMatrix(particle.transform.rotate.y) *
		MakeRotateZMatrix(particle.transform.rotate.z);
	Matrix4x4 translateMatrix = MakeTranslateMatrix(particle.transform.translate);

	if (isDebug) {
		if (particleGroups[name].useBillboard_ && debugCamera_) {
			const Matrix4x4& billBoardMatrix = debugCamera_->GetBillboardMatrix();
			return scaleMatrix * rotateMatrix * billBoardMatrix * translateMatrix;
		} else {
			return scaleMatrix * rotateMatrix * translateMatrix;
		}
	} else {
		if (particleGroups[name].useBillboard_ && camera_) {
			const Matrix4x4& billBoardMatrix = camera_->GetBillboardMatrix();
			return scaleMatrix * rotateMatrix * billBoardMatrix * translateMatrix;
		} else {
			return scaleMatrix * rotateMatrix * translateMatrix;
		}
	}
}

// WVP行列計算関数
Matrix4x4 ParticleManager::CalculateWVPMatrix(const Matrix4x4& worldMatrix, bool isDebug)
{
	if (isDebug) {
		if (debugCamera_) {
			const Matrix4x4& viewProjectionMatrix = debugCamera_->GetViewProjectionMatrix();
			return Multiply(worldMatrix, viewProjectionMatrix);
		} else {
			return worldMatrix;
		}
	} else {
		if (camera_) {
			const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
			return Multiply(worldMatrix, viewProjectionMatrix);
		}
	}

	return worldMatrix;
}

float ParticleManager::RandomRange(std::mt19937& engine, float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(engine);
}

Vector3 ParticleManager::RandomRange(std::mt19937& engine, const Vector3& min, const Vector3& max)
{
	return {
	   RandomRange(engine, min.x, max.x),
	   RandomRange(engine, min.y, max.y),
	   RandomRange(engine, min.z, max.z)
	};
}

Vector4 ParticleManager::RandomRange(std::mt19937& engine, const Vector4& min, const Vector4& max)
{
	return {
		RandomRange(engine, min.x, max.x),
		RandomRange(engine, min.y, max.y),
		RandomRange(engine, min.z, max.z),
		RandomRange(engine, min.w, max.w)
	};
}

void ParticleManager::CreateRingModel()
{
	const uint32_t kRingDivide = 32;
	const float kOuterRadius = 1.0f;
	const float kInnerRadius = 0.2f;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);

	std::vector<VertexData> vertices(kRingDivide * 4);
	std::vector<uint32_t> indices(kRingDivide * 6);

	for (uint32_t index = 0; index < kRingDivide; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float u = float(index) / float(kRingDivide);
		float uNext = float(index + 1) / float(kRingDivide);

		uint32_t vIdx = index * 4;
		// positionとuv。
		vertices[vIdx] = { {-sin * kOuterRadius, cos * kOuterRadius, 0.0f, 1.0f}, {u, 0.0f} };
		vertices[vIdx + 1] = { {-sinNext * kOuterRadius, cosNext * kOuterRadius, 0.0f, 1.0f}, {uNext, 0.0f} };
		vertices[vIdx + 2] = { {-sin * kInnerRadius, cos * kInnerRadius, 0.0f, 1.0f}, {u, 1.0f} };
		vertices[vIdx + 3] = { {-sinNext * kInnerRadius, cosNext * kInnerRadius, 0.0, 1.0f}, {uNext, 1.0f} };

		uint32_t iIdx = index * 6;
		indices[iIdx] = vIdx;
		indices[iIdx + 1] = vIdx + 1;
		indices[iIdx + 2] = vIdx + 2;
		indices[iIdx + 3] = vIdx + 1;
		indices[iIdx + 4] = vIdx + 3;
		indices[iIdx + 5] = vIdx + 2;
	}

	size_t vertexBufferSize = sizeof(VertexData) * vertices.size();

	vertexBufferRing_ = CreateBufferResource(Graphics::GetDevice(), vertexBufferSize);

	// 書き込み
	VertexData* vbPtr = nullptr;
	vertexBufferRing_->Map(0, nullptr, reinterpret_cast<void**>(&vbPtr));
	memcpy(vbPtr, vertices.data(), vertexBufferSize);
	vertexBufferRing_->Unmap(0, nullptr);

	vbViewRing_.BufferLocation = vertexBufferRing_->GetGPUVirtualAddress();
	vbViewRing_.SizeInBytes = (UINT)vertexBufferSize;
	vbViewRing_.StrideInBytes = sizeof(VertexData);

	// インデックスバッファ生成
	size_t indexBufferSize = sizeof(uint32_t) * indices.size();

	indexBufferRing_ = CreateBufferResource(Graphics::GetDevice(), indexBufferSize);

	uint32_t* ibPtr = nullptr;
	indexBufferRing_->Map(0, nullptr, reinterpret_cast<void**>(&ibPtr));
	memcpy(ibPtr, indices.data(), indexBufferSize);
	indexBufferRing_->Unmap(0, nullptr);

	ibViewRing_.BufferLocation = indexBufferRing_->GetGPUVirtualAddress();
	ibViewRing_.SizeInBytes = (UINT)indexBufferSize;
	ibViewRing_.Format = DXGI_FORMAT_R32_UINT;
}

void ParticleManager::CreateCylinderModel()
{
	const uint32_t kCylinderDivide = 32;
	const float kTopRadius = 1.0f;
	const float kBottomRadius = 1.0f;
	const float kHeight = 3.0f;
	const float halfHeight = kHeight * 0.5f;
	const float radiusPerDivide = 2.0f * std::numbers::pi_v<float> / float(kCylinderDivide);

	std::vector<VertexData> vertices(kCylinderDivide * 4);
	std::vector<uint32_t> indices(kCylinderDivide * 6);

	for (uint32_t index = 0; index < kCylinderDivide; ++index) {
		float sin = std::sin(index * radiusPerDivide);
		float cos = std::cos(index * radiusPerDivide);
		float sinNext = std::sin((index + 1) * radiusPerDivide);
		float cosNext = std::cos((index + 1) * radiusPerDivide);
		float u = float(index) / float(kCylinderDivide);
		float uNext = float(index + 1) / float(kCylinderDivide);

		uint32_t vIdx = index * 4;
		// position texcord normal
		vertices[vIdx] = { {-sin * kTopRadius, halfHeight, cos * kTopRadius, 1.0f}, {u, 1.0f}, {-sin, 0.0f, cos } };
		vertices[vIdx + 1] = { {-sinNext * kTopRadius, halfHeight, cosNext * kTopRadius, 1.0f}, {uNext, 1.0f}, {-sinNext, 0.0f, cosNext } };
		vertices[vIdx + 2] = { {-sin * kBottomRadius, -halfHeight, cos * kBottomRadius, 1.0f}, {u, 0.0f}, {-sin, 0.0f, cos } };
		vertices[vIdx + 3] = { {-sinNext * kBottomRadius, -halfHeight, cosNext * kBottomRadius, 1.0f}, {uNext, 0.0f}, {-sinNext, 0.0f, cosNext } };
		//{{-sin * kBottomRadius, 0.0f, cos * kBottomRadius, 1.0f}, {u, 1.0f}, {-sin, 0.0f, cos} }
		//{{-sin * kBottomRadius, 0.0f, cos * kBottomRadius, 1.0f}, {u, 1.0f}, {-sin, 0.0f, cos} }
		//{{-sinNext * kTopRadius, kHeight, cosNext * kTopRadius, 1.0f}, {uNext, 0.0f}, {-sinNext, 0.0f, cosNext} }
		//{{-sinNext * kBottomRadius, 0.0f, cosNext * kBottomRadius, 1.0f}, {uNext, 1.0f}, {-sinNext, 0.0f, cosNext }}

		uint32_t iIdx = index * 6;
		indices[iIdx] = vIdx;
		indices[iIdx + 1] = vIdx + 1;
		indices[iIdx + 2] = vIdx + 2;

		indices[iIdx + 3] = vIdx + 1;
		indices[iIdx + 4] = vIdx + 3;
		indices[iIdx + 5] = vIdx + 2;
	}

	size_t vertexBufferSize = sizeof(VertexData) * vertices.size();

	vertexBufferCylinder_ = CreateBufferResource(Graphics::GetDevice(), vertexBufferSize);

	// 書き込み
	VertexData* vbPtr = nullptr;
	vertexBufferCylinder_->Map(0, nullptr, reinterpret_cast<void**>(&vbPtr));
	memcpy(vbPtr, vertices.data(), vertexBufferSize);
	vertexBufferCylinder_->Unmap(0, nullptr);

	vbViewCylinder_.BufferLocation = vertexBufferCylinder_->GetGPUVirtualAddress();
	vbViewCylinder_.SizeInBytes = (UINT)vertexBufferSize;
	vbViewCylinder_.StrideInBytes = sizeof(VertexData);

	// インデックスバッファ生成
	size_t indexBufferSize = sizeof(uint32_t) * indices.size();

	indexBufferCylinder_ = CreateBufferResource(Graphics::GetDevice(), indexBufferSize);

	uint32_t* ibPtr = nullptr;
	indexBufferCylinder_->Map(0, nullptr, reinterpret_cast<void**>(&ibPtr));
	memcpy(ibPtr, indices.data(), indexBufferSize);
	indexBufferCylinder_->Unmap(0, nullptr);

	ibViewCylinder_.BufferLocation = indexBufferCylinder_->GetGPUVirtualAddress();
	ibViewCylinder_.SizeInBytes = (UINT)indexBufferSize;
	ibViewCylinder_.Format = DXGI_FORMAT_R32_UINT;
}

// グラフィックパイプラインを生成する関数
void ParticleManager::CreateGraphicsPipeline(DxcCompiler& dxcCompiler)
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
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	vsBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/Particle3D.VS.hlsl", L"vs_6_0");
	psBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/Particle3D.PS.hlsl", L"ps_6_0");

	// PSOを生成する
	// 3D用
	PsoBuilder builder;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc3D{};
	builder.Init(graphics_);
	psoDesc3D = builder.CreatePsoDesc(
		rootSignature_,
		inputLayoutDesc3D,
		vsBlob_,
		psBlob_,
		blendDesc_,
		rasterizerDesc,
		depthStencilDesc_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	psoParticle3D_ = builder.BuildPso(psoDesc3D);
	Logger::Write("Particle3DPipelineState生成完了");
}

// 板ポリ生成関数
void ParticleManager::CreatePlaneModel()
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

	vertexBufferPlane_ = CreateBufferResource(Graphics::GetDevice(), vertexBufferSize);

	// 書き込み
	VertexData* vbPtr = nullptr;
	vertexBufferPlane_->Map(0, nullptr, reinterpret_cast<void**>(&vbPtr));
	memcpy(vbPtr, vertices, vertexBufferSize);
	vertexBufferPlane_->Unmap(0, nullptr);

	vbViewPlane_.BufferLocation = vertexBufferPlane_->GetGPUVirtualAddress();
	vbViewPlane_.SizeInBytes = (UINT)vertexBufferSize;
	vbViewPlane_.StrideInBytes = sizeof(VertexData);

	// インデックスバッファ生成
	size_t indexBufferSize = sizeof(indices);

	indexBufferPlane_ = CreateBufferResource(Graphics::GetDevice(), indexBufferSize);

	uint32_t* ibPtr = nullptr;
	indexBufferPlane_->Map(0, nullptr, reinterpret_cast<void**>(&ibPtr));
	memcpy(ibPtr, indices, indexBufferSize);
	indexBufferPlane_->Unmap(0, nullptr);

	ibViewPlane_.BufferLocation = indexBufferPlane_->GetGPUVirtualAddress();
	ibViewPlane_.SizeInBytes = (UINT)indexBufferSize;
	ibViewPlane_.Format = DXGI_FORMAT_R32_UINT;

	Logger::Write("Particle Plane Model Generated");
}

void ParticleManager::DrawParticleGroupImGui(const std::string& name) {
#ifdef USE_IMGUI
	auto& group = ParticleManager::GetInstance()->GetGroup(name);

	ImGui::Text("Group: %s", name.c_str());

	int mode = static_cast<int>(group.blendMode_);
	const char* modes[] = { "None", "Normal", "Add", "Sub", "Mul", "Screen" };
	if (ImGui::Combo("BlendMode", &mode, modes, IM_ARRAYSIZE(modes))) {
		ParticleManager::GetInstance()->SetBlendMode(name, static_cast<BlendMode>(mode));
	}

	float speed[2] = { group.uvScrollSpeed.x, group.uvScrollSpeed.y };
	if (ImGui::DragFloat2("UV Scroll Speed (U, V)", speed, 0.01f, -5.0f, 5.0f)) {
		group.uvScrollSpeed.x = speed[0];
		group.uvScrollSpeed.y = speed[1];
	}

	float alphaReference = group.alphaReference;
	if (ImGui::DragFloat("alphaReference", &alphaReference, 0.01f, -1.0f, 1.0f)) {
		group.alphaReference = alphaReference;
	}

	ImGui::Checkbox("Billboard", &group.useBillboard_);
#endif
}

uint32_t ParticleManager::GetTotalParticleCount() const
{
	uint32_t totalCount = 0;
	for (const auto& [name, group] : particleGroups) {
		totalCount += static_cast<uint32_t>(group.particles.size());
	}
	return totalCount;
}

uint32_t ParticleManager::GetParticleGroupCount() const
{
	return static_cast<uint32_t>(particleGroups.size());;
}

ParticleModelType ParticleManager::GetModelType(const std::string& name)
{
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end() && "Particle group not found");
	return it->second.modelType_;
}

json ParticleManager::SaveToJson(const std::string& name) const {
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end() && "Particle group not found");

	return json{
		{"blendMode", it->second.blendMode_},
		{"billboard", it->second.useBillboard_},
		{"modelType", it->second.modelType_},
		{"scrollSpeedX", it->second.uvScrollSpeed.x },
		{"scrollSpeedY", it->second.uvScrollSpeed.y },
		{"alphaReference", it->second.alphaReference }
	};
}

void ParticleManager::LoadFromJson(const json& j, const std::string& name) {
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end() && "Particle group not found");

	if (j.contains("blendMode")) {
		it->second.blendMode_ = static_cast<BlendMode>(j.value("blendMode", it->second.blendMode_));
	}

	if (j.contains("billboard")) {
		it->second.useBillboard_ = static_cast<bool>(j.value("billboard", it->second.useBillboard_));
	}

	if (j.contains("modelType")) {
		it->second.modelType_ = static_cast<ParticleModelType>(j.value("modelType", it->second.modelType_));
	}

	if (j.contains("scrollSpeedX")) {
		it->second.uvScrollSpeed.x = j.value("scrollSpeedX", it->second.uvScrollSpeed.x);
	}

	if (j.contains("scrollSpeedY")) {
		it->second.uvScrollSpeed.y = j.value("scrollSpeedY", it->second.uvScrollSpeed.y);
	}

	if (j.contains("alphaReference")) {
		it->second.alphaReference = j.value("alphaReference", it->second.alphaReference);
	}
}
