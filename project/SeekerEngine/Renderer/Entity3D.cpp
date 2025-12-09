#include "Entity3D.h"
#include "Entity3DCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"

void Entity3D::Init(Entity3DCommon* entity3DCommon)
{
	this->entity3DCommon_ = entity3DCommon;
	this->camera_ = entity3DCommon->GetDefaultCamera();
	this->debugCamera_ = entity3DCommon->GetDebugCamera();
	this->cameraManager_ = entity3DCommon->GetCameraManager();
	cmdList_ = entity3DCommon->GetCmdList();
	mode_ = entity3DCommon->GetBlendMode();
	ModelResourcesSetting();

	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
}

void Entity3D::Update()
{
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	// WVPMatrixを作る
	Matrix4x4 worldViewProjectionMatrix;

	bool isDebug = cameraManager_->GetIsDebug();

	if (isDebug) {
		if (debugCamera_) {
			const Matrix4x4& viewProjectionMatrix = debugCamera_->GetViewProjectionMatrix();
			worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
		} else {
			worldViewProjectionMatrix = worldMatrix;
		}
	} else {
		if (camera_) {
			const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
			worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
		} else {
			worldViewProjectionMatrix = worldMatrix;
		}
	}

	transformationMatrixData_->World = worldMatrix;
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
}

void Entity3D::Draw()
{
	// wvp用のCBufferの場所を設定
	cmdList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	cmdList_->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	if (model_) {
		model_->Draw();
	}
}

void Entity3D::SetModel(const std::string& filePath)
{
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}

void Entity3D::SetBlendMode(BlendMode mode)
{
	this->mode_ = mode;
	entity3DCommon_->SetBlendMode(mode);
}

void Entity3D::ModelResourcesSetting()
{
	// TransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	// 単位行列を書きこんでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();

	// 平行光源用のリソース
	directionalLightResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	// 初期化値
	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 1.0f, 0.0f, 0.0f };
	directionalLightData_->intensity = 1.0f;
}