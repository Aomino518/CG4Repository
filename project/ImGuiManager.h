#pragma once
#include "Graphics.h"
#include "Application.h"
#include <stdlib.h>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif

class ImGuiManager
{
public:
	void Init(Application* app, Graphics* graphics);
	void BegineFrame();
	void EndFrame();
	void Draw();
	void Shutdown();

	void SpriteSetting(const std::string& spriteName, const Vector4& spriteMaterial, const Vector2& positoin, float& rotation, const Vector2& scale);
	void ModelSetting(const std::string& modelName, Vector4& spriteMaterial, Vector3& positoin, Vector3& rotation, Vector3& scale);
	void Stats();
	void ShowMemoryUsage();
	void BegineInspector();
	void EndInspector();
	void CameraSetting(Vector3& positoin, Vector3& rotation);

private:
	void StyleSetting();

	Application* app_ = nullptr;
	Graphics* graphics_ = nullptr;
};

