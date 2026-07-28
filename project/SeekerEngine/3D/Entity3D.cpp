#include "Entity3D.h"
#include "Entity3DCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "JsonTransform.h"
#include "JsonMath.h"
#include "CreateResorceUtils.h"
#include "MathFunc.h"
#include "DebugDraw.h"

void Entity3D::Init(const std::string& filePath, bool isSkinned)
{
	this->camera_ = Entity3DCommon::GetInstance()->GetDefaultCamera();
	this->debugCamera_ = Entity3DCommon::GetInstance()->GetDebugCamera();
	this->cameraManager_ = Entity3DCommon::GetInstance()->GetCameraManager();
	cmdList_ = Entity3DCommon::GetInstance()->GetCmdList();
	mode_ = Entity3DCommon::GetInstance()->GetBlendMode();

	// モデルリソース作成
	CreateModelResources();
	// 使用モデル検索
	model_ = ModelManager::GetInstance()->FindModel(filePath);
	// アニメーションを取得
	animation_ = model_->GetAnimation();
	// Skinningフラグ取得
	isSkinned_ = isSkinned;
	// SkinningのときレンダータイプをSkinningに
	if (isSkinned_) {
		renderType_ = ModelRenderType::Skinning;
	}

	if (renderType_ == ModelRenderType::Skinning) {
		// スケルトンを作成
		skeleton_ = model_->CreateSkeleton(model_->GetRootNode().rootNode);
		// スキンクラスターを作成
		skinCluster_ = model_->CreateSkinCluster(skeleton_, model_->GetRootNode());
	}

	// 初期位置
	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
}

void Entity3D::Update()
{
	// RootNode取得
	ModelData modelData = model_->GetRootNode();
	bool isDebug = cameraManager_->GetIsDebug();

	// worldMatrixを作る
	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	// WVPMatrixを作る
	Matrix4x4 viewProjectionMatrix = MakeIdentity4x4();;

	// デバッグカメラとゲームカメラの行列切り替え
	if (isDebug) {
		if (debugCamera_) {
			viewProjectionMatrix = debugCamera_->GetViewProjectionMatrix();
		}
	} else {
		if (camera_) {
			viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		}
	}

	if (isAnimationPlaying_ && animation_.duration > 0.0f && !animation_.nodeAnimations.empty()) {
		animationTime_ += 1.0f / 60.0f;
		animationTime_ = std::fmod(animationTime_, animation_.duration);
	}

	// スキニングのときはskeleton,skinningを更新
	if (renderType_ == ModelRenderType::Skinning) {
		model_->ApplyAnimation(skeleton_, animation_, animationTime_);
		model_->UpdateSkeleton(skeleton_);
		model_->UpdateSkinCluster(skinCluster_, skeleton_);
	}

	auto keyframe = animation_.nodeAnimations.find(modelData.rootNode.name);
	// モデルにアニメーションがある場合アニメーションする
	if (keyframe != animation_.nodeAnimations.end()) {
		NodeAnimation& rootNodeAnimation = animation_.nodeAnimations[modelData.rootNode.name];
		Vector3 rootTranslate = CalculateValue(rootNodeAnimation.translate.keyframes, animationTime_);
		Quaternion rootRotate = CalculateValue(rootNodeAnimation.rotate.keyframes, animationTime_);
		Vector3 rootScale = CalculateValue(rootNodeAnimation.scale.keyframes, animationTime_);
		Matrix4x4 localMatrix = MakeAffineMatrix(rootScale, rootRotate, rootTranslate);
		finalWorldMatrix_ = localMatrix * worldMatrix_;
	} else {
		finalWorldMatrix_ = worldMatrix_;
	}

	// 最終的な行列の適用
	transformationMatrixData_->World = finalWorldMatrix_;
	transformationMatrixData_->WVP = finalWorldMatrix_ * viewProjectionMatrix;
	transformationMatrixData_->WorldInverseTranspose = Transpose(Inverse(finalWorldMatrix_));

	// スペキュラー反射計算用に、現在使用中のカメラのワールド座標をシェーダーへ渡す
	if (cameraData_) {
		if (isDebug && debugCamera_) {
			cameraData_->worldPosition = debugCamera_->GetTranslate();
		} else if (camera_) {
			cameraData_->worldPosition = camera_->GetTranslate();
		}
	}
}

void Entity3D::Draw()
{
	Entity3DCommon::GetInstance()->SetBlendMode(mode_, renderType_);
	Entity3DCommon::GetInstance()->ApplyPipeline(renderType_);

	// wvp用のCBufferの場所を設定
	cmdList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	cmdList_->SetGraphicsRootConstantBufferView(3, LightManager::GetInstance()->GetDirLightResource()->GetGPUVirtualAddress());
	cmdList_->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
	cmdList_->SetGraphicsRootConstantBufferView(5, LightManager::GetInstance()->GetPointLightGroupResource()->GetGPUVirtualAddress());
	cmdList_->SetGraphicsRootConstantBufferView(6, LightManager::GetInstance()->GetSpotLightGroupResource()->GetGPUVirtualAddress());

	if (model_) {
		if (isSkinned_) {
			model_->DrawSkinning(skinCluster_);
		} else {
			model_->Draw();
		}
	}
}

json Entity3D::SaveToJson() const
{
	return json{
	   {"transform", TransformToJson(transform_)},
	   {"blendMode", static_cast<int>(mode_)},
	   {"material", ToJson(model_->GetMaterial())},
	   {"environmentColor", model_->GetEnvironmentColor()}
	};
}

void Entity3D::LoadFromJson(const json& j)
{
	if (j.contains("transform")) {
		TransformFromJson(j.at("transform"), transform_);
	}

	if (j.contains("blendMode")) {
		mode_ = static_cast<BlendMode>(j.at("blendMode").get<int>());
	}

	if (j.contains("material")) {
		Vector4 material{};
		FromJson(j.at("material"), material);
		SetMaterial(material);
	}

	if (j.contains("environmentColor")) {
		float environmentColor = model_->GetEnvironmentColor();
		float result = j.value("environmentColor", environmentColor);
		model_->SetEnvironmentColor(result);
	}
}

void Entity3D::SetModel(const std::string& filePath)
{
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}

void Entity3D::SetBlendMode(BlendMode mode)
{
	this->mode_ = mode;
	Entity3DCommon::GetInstance()->SetBlendMode(mode, renderType_);
}

// モデルリソース作成関数
void Entity3D::CreateModelResources()
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

void Entity3D::SetAnimationPlaying(bool isPlaying)
{
	isAnimationPlaying_ = isPlaying;
	if (!isAnimationPlaying_) {
		animationTime_ = 0.0f;
	}
}

// ボーン描画する関数
void Entity3D::DrawBorn()
{
	for (const Joint& joint : skeleton_.joints) {
		Matrix4x4 jointWorldMatrix = joint.skeletonSpaceMatrix * worldMatrix_;
		DebugDraw::DrawBox(GetMatrix4x4Translate(jointWorldMatrix), Vector3{ 0.01f, 0.01f, 0.01f }, Color::WHITE, DebugDrawMode::Wireframe);

		if (joint.parent) {
			const Joint& parent = skeleton_.joints[*joint.parent];
			Matrix4x4 parentWorldMatrix = parent.skeletonSpaceMatrix * worldMatrix_;
			Vector3 childPos = GetMatrix4x4Translate(jointWorldMatrix);
			Vector3 parentPos = GetMatrix4x4Translate(parentWorldMatrix);

			DebugDraw::DrawLine(parentPos, childPos, Color::WHITE);
		}
	}
}

// ImGuiの設定情報表示
void Entity3D::DrawImGui() {
#ifdef USE_IMGUI
	Vector4 material = model_->GetMaterial();
	float environmentColor = model_->GetEnvironmentColor();
	ImGui::DragFloat3("Position", reinterpret_cast<float*>(&transform_.translate), 0.01f);
	ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&transform_.rotate), 0.01f);
	ImGui::DragFloat3("Scale", reinterpret_cast<float*>(&transform_.scale), 0.01f, 0.0f);
	ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&material));
	ImGui::DragFloat("EnvironmentColor", reinterpret_cast<float*>(&environmentColor), 0.01f, 0.0f, 1.0f);
	model_->SetMaterial(material);
	model_->SetEnvironmentColor(environmentColor);
#endif
}