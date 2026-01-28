#include "Entity3D.h"
#include "Entity3DCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"

void Entity3D::Init()
{
	this->camera_ = Entity3DCommon::GetInstance()->GetDefaultCamera();
	this->debugCamera_ = Entity3DCommon::GetInstance()->GetDebugCamera();
	this->cameraManager_ = Entity3DCommon::GetInstance()->GetCameraManager();
	cmdList_ = Entity3DCommon::GetInstance()->GetCmdList();
	mode_ = Entity3DCommon::GetInstance()->GetBlendMode();
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

	Matrix4x4 worldInverseTransform = Inverse(worldMatrix);

	transformationMatrixData_->World = worldMatrix;
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->WorldInverseTranspose = worldInverseTransform;
}

void Entity3D::Draw()
{
	// wvp用のCBufferの場所を設定
	cmdList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	cmdList_->SetGraphicsRootConstantBufferView(3, LightManager::GetInstance()->GetDirLightResource()->GetGPUVirtualAddress());
	cmdList_->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
	cmdList_->SetGraphicsRootConstantBufferView(5, LightManager::GetInstance()->GetPointLightResource()->GetGPUVirtualAddress());
	cmdList_->SetGraphicsRootConstantBufferView(6, LightManager::GetInstance()->GetSpotLightResource()->GetGPUVirtualAddress());

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
	Entity3DCommon::GetInstance()->SetBlendMode(mode);
}

void Entity3D::ModelResourcesSetting()
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

	Vector3 camPos = cameraManager_->GetActiveCamera()->GetTranslate();
	cameraData_->worldPosition = camPos;
}