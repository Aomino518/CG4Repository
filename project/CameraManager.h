#pragma once
#include "Camera.h"
#include <memory>
#include <vector>
#include <string>

class CameraManager
{
public:
	void Init();

private:
	std::vector<std::unique_ptr<Camera>> cameras_;
};

