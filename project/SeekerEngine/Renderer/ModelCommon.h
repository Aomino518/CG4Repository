#pragma once
#include "Graphics.h"

class ModelCommon
{
public:
	void Init(Graphics* graphics);

	// getter関数
	Graphics* GetGraphics() const { return graphics_; };

private:
	Graphics* graphics_;
};

