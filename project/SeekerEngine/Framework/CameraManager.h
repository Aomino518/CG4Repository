#pragma once
#include "Camera.h"
#include "DebugCamera.h"
#include <memory>
#include <vector>
#include <string>

class CameraManager
{
public:
	struct CameraInfo {
		std::string name;
		std::unique_ptr<Camera> camera;
	};

	void Init();
	void Update();

	void CreateCamera(const std::string& cameraName);
	void CreateDebugCamera();

	void SetActiveCamera(bool isDebug, int index = 0);

	const std::vector<CameraInfo>& GetCameras() const { return cameras_; }
	Camera* GetActiveCamera() const;
	bool GetIsDebug() const { return activeIsDebug_; }
	int GetActtiveCamIndex() const { return activeCamIndex_; }
	DebugCamera* GetDebugCamera() const { return debugCamera_.get(); }

private:
	std::vector<CameraInfo> cameras_;
	std::unique_ptr<DebugCamera> debugCamera_;

	bool activeIsDebug_ = false;
	int activeCamIndex_ = 0;
};

