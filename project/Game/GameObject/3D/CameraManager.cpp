#include "CameraManager.h"
#include <numbers>

CameraManager* CameraManager::GetInstance() {
	static CameraManager instance;
	return &instance;
}

void CameraManager::Init()
{
	CreateCamera("MainCamera");
	cameras_[0].camera->SetTranslate({0.0f, 23.0f, -10.0f});
	cameras_[0].camera->SetRotate(
		{ std::numbers::pi_v<float> / 3.0f,
		 0.0f,
		 0.0f }
	);
	cameras_[0].camera->SetScale({ 1.0f, 1.0f, 1.0f });
	CreateDebugCamera();
	SetActiveCamera(false, 0);
}

void CameraManager::Update()
{
	if (activeIsDebug_) {
		debugCamera_->Update();
	} else {
		cameras_[activeCamIndex_].camera->Update();
	}
}

void CameraManager::Shutdown()
{
	cameras_.clear();
	debugCamera_.reset();
}	

void CameraManager::CreateCamera(const std::string& cameraName)
{
	CameraInfo cameraInfo;
	cameraInfo.name = cameraName;
	cameraInfo.camera = std::make_unique<Camera>();
	cameraInfo.camera->SetTranslate({0.0f, 10.0f, -10.0f});
	cameraInfo.camera->SetRotate({0.0f, 0.0f, 0.0f});
	cameras_.push_back(std::move(cameraInfo));
}

void CameraManager::CreateDebugCamera()
{
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
}

void CameraManager::SetActiveCamera(bool isDebug, int index)
{
	activeIsDebug_ = isDebug;
	activeCamIndex_ = index;
}

void CameraManager::SetActiveCameraByName(const std::string& name)
{
	for (int i = 0; i < cameras_.size(); ++i) {
		if (cameras_[i].name == name) {
			activeCamIndex_ = i;
			activeIsDebug_ = false;
			return;
		}
	}
	assert(false && "指定された名前のカメラが存在しません");
}

Camera* CameraManager::GetActiveCamera() const
{
	if (activeIsDebug_) {
		return (Camera*)debugCamera_.get();
	}

	return  cameras_[activeCamIndex_].camera.get();
}

Camera* CameraManager::GetCamera(const std::string& name)
{
	for(auto& cam : cameras_) {
		if (cam.name == name) {
			return cam.camera.get();
		}
	}

	assert(false && "指定された名前のカメラが見つかりません");
	return nullptr;
}
