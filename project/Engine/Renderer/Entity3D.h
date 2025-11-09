#pragma once
#include <string>
#include <vector>
#include <d3d12.h>
#include <wrl.h>
#include <cassert>  
#include "Matrix.h"
#include <unordered_map>
#include <algorithm>
#include "CreateResorceUtils.h"
#include "Model.h"
#include "Camera.h"

class Entity3DCommon;
class TextureManager;

class Entity3D
{
public:
	void Init(Entity3DCommon* entity3DCommon);

	void Update();

	void Draw();

	// getter関数
	const Vector3& GetScale() const { return transform_.scale; }
	const Vector3& GetRotate() const { return transform_.rotate; }
	const Vector3& GetTranslate() const { return transform_.translate; }
	const Vector4& GetMaterial() const { return model_->GetMaterial(); }

	// setter関数
	void SetModel(const std::string& filePath);
	void SetScale(const Vector3& scale) { this->transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { this->transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { this->transform_.translate = translate; }
	void SetCamera(Camera* camera) { this->camera_ = camera; }
	void SetMaterial(const Vector4& material) { model_->SetMaterial(material); }

private:
	void ModelResourcesSetting();

	Transform transform_;

	Entity3DCommon* entity3DCommon_ = nullptr;
	
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;

	Model* model_ = nullptr;
	Camera* camera_ = nullptr;
};

