#pragma once
#include "Graphics.h"
#include "Application.h"
#include <stdlib.h>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Sprite.h"
#include "CameraManager.h"
#include "BlendStateUtils.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif

class Entity3D;
class ImGuiManager
{
public:
	static ImGuiManager* GetInstance();

	void Init(Application* app, Graphics* graphics);
	void BegineFrame();
	void EndFrame();
	void Draw();
	void Shutdown();

	void SpriteSetting(const std::string& spriteName, Sprite* sprite);
	void ModelSetting(const std::string& modelName, Entity3D* model);
	void Stats();
	void ShowMemoryUsage();
	void BegineInspector();
	void EndInspector();
	void CameraSetting(CameraManager* cameraManager);
	void ParticleSetting(const std::string& name, ParticleEmitter* emitter);
	void LightSetting();
	void SoundSetting();

private:
	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;

	void StyleSetting();

	Application* app_ = nullptr;
	Graphics* graphics_ = nullptr;
};

