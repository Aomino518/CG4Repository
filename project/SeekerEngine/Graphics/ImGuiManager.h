#pragma once
#include <stdlib.h>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Sprite.h"
#include "CameraManager.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "ImGuiUtils.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif

class Application;
class Graphics;
class Entity3D;
class ImGuiManager
{
public:
	static ImGuiManager* GetInstance();

	void Init();
	void BeginFrame();
	void EndFrame();
	void Draw();
	void Shutdown();

	void DrawSpriteInspector(const std::string& spriteName, Sprite* sprite);
	void DrawModelInspector(const std::string& modelName, Entity3D* model);
	void DrawParticleInspector(const std::string& name, ParticleEmitter* emitter);
	void DrawLightWindow();
	void DrawSoundWindow();
	void DrawCameraWindow(CameraManager* cameraManager);

	void Stats();
	void ShowMemoryUsage();
	void BeginInspector();
	void EndInspector();

private:
	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;

	// メンバ関数
	void ApplyStyle();

	Application* app_ = nullptr;
	Graphics* graphics_ = nullptr;
};

