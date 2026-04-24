#pragma once
#include <stdlib.h>
#include <string>
#include <nlohmann/json.hpp>
#ifdef USE_IMGUI
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#endif

class Application;
class Graphics;
class Entity3D;
class Sprite;
class CameraManager;
class ParticleManager;
class ParticleEmitter;
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
	void DrawSoundWindow();
	void DrawCameraWindow(CameraManager* cameraManager);
	void DrawMainMenuBar();
	void DrawLoggerWindow();

	void DrawEditor();

	void Stats();
	void ShowMemoryUsage();
	void BeginInspector();
	void EndInspector();
	void SaveScenesJson();
	void LoadScenesJson();

	nlohmann::json SaveEditorJson() const;
	void LoadEditorJson(const nlohmann::json& j);

private:
	// メンバ関数
	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;

	void ApplyStyle();

	// メンバ変数
	Application* app_ = nullptr;
	Graphics* graphics_ = nullptr;

	// 表示するWindowのフラグ
	struct WindowFlags {
		bool showStats = true;
		bool showCamera = true;
		bool showSound = true;
		bool showLogger = true;
	};

	WindowFlags windowState_;
};

