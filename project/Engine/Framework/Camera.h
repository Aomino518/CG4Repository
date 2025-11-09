#pragma once
#include "CreateResorceUtils.h"
#include "Application.h"

class Camera
{
public:
	Camera();
	void Update();

	// Getter
	const Vector3& GetTranslate() const { return transform_.translate; }
	const Vector3& GetRotate() const { return transform_.rotate; }
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }

	// Setter
	void SetTranslate(const Vector3& translate) { this->transform_.translate = translate; }
	void SetRotate(const Vector3& rotate) { this->transform_.rotate = rotate; }
	void SetFovY(float fovY) { this->fovY_ = fovY; }
	void SetAspectRatio(float aspectRatio) { this->aspectRatio_ = aspectRatio; }
	void SetNearClip(float nearClip) { this->nearClip_ = nearClip; }
	void SetFarClip(float farClip) { this->farClip_ = farClip; }

private:
	Transform transform_;
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;
	float fovY_;
	float aspectRatio_;
	float nearClip_;
	float farClip_;
};

