#include "ImGuiManager.h"
#include "ImGuiUtils.h"
#include "Logger.h"
#include <Psapi.h>
#include "Entity3D.h"
#include "Sprite.h"
#include "ParticleEmitter.h"
#include "Application.h"
#include "Graphics.h"
#include "ModelManager.h"
#include "SoundManager.h"
#include "LightManager.h"
#include "EmitterManager.h"
#include "SceneManager.h"
#include "Editor.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <nlohmann/json.hpp>

ImGuiManager* ImGuiManager::GetInstance() {
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::Init()
{
#ifdef USE_IMGUI
	app_ = Application::GetInstance();
	graphics_ = Graphics::GetInstance();

	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	io.Fonts->AddFontFromFileTTF(
		"resources/fonts/NotoSansJP-Regular.ttf",
		17.0f,
		nullptr,
		io.Fonts->GetGlyphRangesJapanese()
	);

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(app_->GetHWND());
	ImGui_ImplDX12_Init(graphics_->GetDevice(),
		graphics_->GetSwapChainDesc().BufferCount,
		graphics_->GetRTVDesc().Format,
		SrvManager::GetInstance()->GetDiscriptorHeap(),
		SrvManager::GetInstance()->GetDiscriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		SrvManager::GetInstance()->GetDiscriptorHeap()->GetGPUDescriptorHandleForHeapStart()
	);
	Logger::Write("ImGui初期化");

	// スタイルを設定
	ApplyStyle();
#endif
}

void ImGuiManager::BeginFrame()
{
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif
}

void ImGuiManager::EndFrame()
{
#ifdef USE_IMGUI
	// ImGuiの内部コマンドを生成する
	ImGui::Render();
#endif
}

void ImGuiManager::Draw()
{
#ifdef USE_IMGUI
	// 実際のcommandListのImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), graphics_->GetCmdList());
#endif
}

void ImGuiManager::Shutdown()
{
#ifdef USE_IMGUI
	// ImGuiの終了処理。初期化と逆順に行う
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	Logger::Write("ImGuiManager Shutdown");
#endif
}

void ImGuiManager::DrawSpriteInspector(const std::string& spriteName, Sprite* sprite)
{
#ifdef USE_IMGUI
	Vector4 spriteMaterial = sprite->GetColor();
	Vector2 spritePosition = sprite->GetPosition();
	float spriteRotate = sprite->GetRotation();
	Vector2 spriteScale = sprite->GetSize();

	if (ImGui::CollapsingHeader(spriteName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushID(spriteName.c_str());
		if (ImGuiUtils::DrawColor4("Material", spriteMaterial)) {
			sprite->SetColor(spriteMaterial);
		}

		if (ImGuiUtils::DrawTransform2D(spritePosition, spriteRotate, spriteScale)) {
			sprite->SetPosition(spritePosition);
			sprite->SetRotation(spriteRotate);
			sprite->SetSize(spriteScale);
		}

		BlendMode blendMode = sprite->GetBlendMode();
		if (ImGuiUtils::DrawBlendModeSelector("BlendMode", blendMode)) {
			sprite->SetBlendMode(blendMode);
		}

		ImGui::PopID();
	}
#endif
}

void ImGuiManager::DrawModelInspector(const std::string& modelName, Entity3D* model)
{
#ifdef USE_IMGUI
	Vector4 modelMaterial = model->GetMaterial();
	Vector3 modelPosition = model->GetTranslate();
	Vector3 modelRotate = model->GetRotate();
	Vector3 modelScale = model->GetScale();

	if (ImGui::CollapsingHeader(modelName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushID(modelName.c_str());
		if (ImGuiUtils::DrawColor4("Material", modelMaterial)) {
			model->SetMaterial(modelMaterial);
		}

		if (ImGuiUtils::DrawTransform3D(modelPosition, modelRotate, modelScale)) {
			model->SetTranslate(modelPosition);
			model->SetRotate(modelRotate);
			model->SetScale(modelScale);
		}

		BlendMode blendMode = model->GetBlendMode();
		if (ImGuiUtils::DrawBlendModeSelector("BlendMode", blendMode)) {
			model->SetBlendMode(blendMode);
		}

		ImGui::PopID();
	}

#endif
}

void ImGuiManager::Stats()
{
#ifdef USE_IMGUI
	if (windowState_.showStats) {
		ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::SeparatorText("Frame");
		float fps = ImGui::GetIO().Framerate;
		ImGui::Text("FPS: %.2f", fps);
		ImGui::Text("Frame Time: %.2f ms", 1000.0f / fps);

		ImGui::SeparatorText("Rendering");
		ImGui::Text("DrawCalls: %u", Graphics::GetInstance()->GetDrawCallCount());

		ImGui::SeparatorText("Particle");
		ImGui::Text("Particles: %u", ParticleManager::GetInstance()->GetTotalParticleCount());
		ImGui::Text("Particle Groups: %u", ParticleManager::GetInstance()->GetParticleGroupCount());
		ImGui::Text("Emitters: %u", EmitterManager::GetInstance()->GetEmitterCount());

		ImGui::SeparatorText("Memory");
		ShowMemoryUsage();
		ImGui::End();
	}
#endif
}

void ImGuiManager::ShowMemoryUsage() {
#ifdef USE_IMGUI
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
		// メモリ使用量をMB単位で計算
		double memoryUsageMB = pmc.WorkingSetSize / (1024.0 * 1024.0);

		ImGui::Text("Memory Usage: %.2f MB", memoryUsageMB);
	}
#endif
}

void ImGuiManager::BeginInspector()
{
#ifdef USE_IMGUI
	ImGui::Begin("Inspector");
#endif
}

void ImGuiManager::EndInspector()
{
#ifdef USE_IMGUI
	ImGui::End();
#endif
}

void ImGuiManager::DrawCameraWindow(CameraManager* cameraManager)
{
#ifdef USE_IMGUI
	if (windowState_.showCamera) {
		ImGui::Begin("Camera Manager");

		if (ImGui::CollapsingHeader("Debug Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool selected = cameraManager->GetIsDebug();
			if (ImGui::Selectable("DebugCamera", selected)) {
				cameraManager->SetActiveCamera(true);
			}
		}

		if (ImGui::CollapsingHeader("Normal Cameras", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& cameras = cameraManager->GetCameras();

			for (int i = 0; i < cameras.size(); i++)
			{
				bool selected = (!cameraManager->GetIsDebug() && cameraManager->GetActtiveCamIndex() == i);
				Vector3 pos = cameras[i].camera->GetTranslate();
				Vector3 rot = cameras[i].camera->GetRotate();

				if (ImGui::Selectable(cameras[i].name.c_str(), selected))
				{
					cameraManager->SetActiveCamera(false, i);
				}

				// 展開してパラメータ編集
				if (selected)
				{
					ImGui::PushID(cameras[i].name.c_str());
					ImGui::Indent();

					bool changed = false;
					changed |= ImGui::DragFloat3("Position", (float*)&pos, 0.01f);
					changed |= ImGui::DragFloat3("Rotation", (float*)&rot, 0.01f);

					if (changed) {
						cameras[i].camera->SetTranslate(pos);
						cameras[i].camera->SetRotate(rot);
					}

					ImGui::Unindent();
					ImGui::PopID();
				}
			}
		}

		ImGui::End();
	}

#endif
}

void ImGuiManager::DrawMainMenuBar()
{
#ifdef USE_IMGUI
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save")) {
				SaveScenesJson();
			}

			if (ImGui::MenuItem("Load")) {
				LoadScenesJson();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window")) {
			ImGui::MenuItem("Stats", nullptr, &windowState_.showStats);
			ImGui::MenuItem("Camera", nullptr, &windowState_.showCamera);
			ImGui::MenuItem("Sound", nullptr, &windowState_.showSound);
			ImGui::MenuItem("Logger", nullptr, &windowState_.showLogger);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
#endif
}

void ImGuiManager::DrawLoggerWindow()
{
#ifdef USE_IMGUI
	if (windowState_.showLogger) {
		ImGui::Begin("Console");

		if (ImGui::Button("Clear")) {
			Logger::ClearHistory();
		}

		ImGui::SameLine();

		static bool autoScroll = true;
		ImGui::Checkbox("Auto Scroll", &autoScroll);

		ImGui::Separator();

		ImGui::BeginChild("Log Region", ImVec2(0, 0), true);

		const auto logs = Logger::GetHistory();
		for (const auto& log : logs) {
			ImGui::TextUnformatted(log.c_str());
		}

		if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f) {
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();
		ImGui::End();
	}
#endif
}

void ImGuiManager::DrawEditor()
{
	Editor::GetInstance()->Draw();
}

void ImGuiManager::DrawSoundWindow()
{
#ifdef USE_IMGUI
	if (windowState_.showSound) {
		ImGui::Begin("Sound Manager");
		SoundManager* soundManager = SoundManager::GetInstance();
		auto volumeBGM = soundManager->GetCurrentBGMVolume();
		auto volumeSE = soundManager->GetCurrentSEVolume();
		auto volumeMaster = soundManager->GetCurrentMasterVolume();

		if (ImGui::DragFloat("BGM Volume", &volumeBGM, 0.01f, 0.0f, 1.0f, "%.2f")) {
			soundManager->SetVolumeBGM(volumeBGM);
		}

		if (ImGui::DragFloat("SE Volume", &volumeSE, 0.01f, 0.0f, 1.0f, "%.2f")) {
			soundManager->SetVolumeSE(volumeSE);
		}

		if (ImGui::DragFloat("Master Volume", &volumeMaster, 0.01f, 0.0f, 1.0f, "%.2f")) {
			soundManager->SetVolumeMaster(volumeMaster);
		}

		ImGui::End();
	}
#endif
}

void ImGuiManager::ApplyStyle()
{
#ifdef USE_IMGUI
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* c = style.Colors;

	// --- 基本背景・テキスト ---
	c[ImGuiCol_Text] = ImVec4(0.90f, 0.96f, 1.00f, 1.00f);
	c[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.55f, 0.65f, 1.00f);

	c[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.06f, 0.09f, 1.00f);
	c[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.10f, 0.14f, 1.00f);
	c[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.11f, 0.15f, 0.98f);

	c[ImGuiCol_Border] = ImVec4(0.16f, 0.34f, 0.48f, 0.85f);
	c[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// --- フレーム（入力欄、スライダーなど） ---
	c[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.16f, 0.22f, 1.00f);
	c[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.24f, 0.33f, 1.00f);
	c[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.31f, 0.42f, 1.00f);

	// --- タイトルバー・ウィンドウ装飾 ---
	c[ImGuiCol_TitleBg] = ImVec4(0.03f, 0.15f, 0.22f, 1.00f);
	c[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.32f, 0.46f, 1.00f);
	c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.03f, 0.12f, 0.18f, 0.75f);
	c[ImGuiCol_MenuBarBg] = ImVec4(0.05f, 0.12f, 0.18f, 1.00f);

	// --- ボタン ---
	c[ImGuiCol_Button] = ImVec4(0.00f, 0.42f, 0.60f, 1.00f);
	c[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.56f, 0.80f, 1.00f);
	c[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.68f, 0.95f, 1.00f);

	// --- ヘッダー（ツリーノード、リスト選択など） ---
	c[ImGuiCol_Header] = ImVec4(0.00f, 0.38f, 0.54f, 0.95f);
	c[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.50f, 0.72f, 1.00f);
	c[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.62f, 0.88f, 1.00f);

	// --- スクロールバー ---
	c[ImGuiCol_ScrollbarBg] = ImVec4(0.03f, 0.05f, 0.07f, 1.00f);
	c[ImGuiCol_ScrollbarGrab] = ImVec4(0.12f, 0.25f, 0.35f, 1.00f);
	c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.16f, 0.38f, 0.52f, 1.00f);
	c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.20f, 0.50f, 0.68f, 1.00f);

	// --- スライダー ---
	c[ImGuiCol_SliderGrab] = ImVec4(0.15f, 0.72f, 1.00f, 1.00f);
	c[ImGuiCol_SliderGrabActive] = ImVec4(0.35f, 0.82f, 1.00f, 1.00f);

	// --- チェックマーク ---
	c[ImGuiCol_CheckMark] = ImVec4(0.20f, 0.80f, 1.00f, 1.00f);

	// --- セパレーター ---
	c[ImGuiCol_Separator] = ImVec4(0.18f, 0.36f, 0.50f, 0.85f);
	c[ImGuiCol_SeparatorHovered] = ImVec4(0.28f, 0.52f, 0.72f, 1.00f);
	c[ImGuiCol_SeparatorActive] = ImVec4(0.38f, 0.68f, 0.90f, 1.00f);

	// --- リサイズグリップ ---
	c[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.52f, 0.74f, 0.55f);
	c[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.66f, 0.92f, 0.80f);
	c[ImGuiCol_ResizeGripActive] = ImVec4(0.20f, 0.80f, 1.00f, 1.00f);

	// --- タブ ---
	c[ImGuiCol_Tab] = ImVec4(0.06f, 0.20f, 0.28f, 1.00f);
	c[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.45f, 0.65f, 1.00f);
	c[ImGuiCol_TabActive] = ImVec4(0.00f, 0.58f, 0.82f, 1.00f);
	c[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.13f, 0.18f, 1.00f);
	c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.26f, 0.36f, 1.00f);

	// --- ドッキング・テーブル ---
	c[ImGuiCol_TableHeaderBg] = ImVec4(0.06f, 0.18f, 0.26f, 1.00f);
	c[ImGuiCol_TableBorderStrong] = ImVec4(0.18f, 0.40f, 0.56f, 1.00f);
	c[ImGuiCol_TableBorderLight] = ImVec4(0.10f, 0.24f, 0.34f, 1.00f);
	c[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	c[ImGuiCol_TableRowBgAlt] = ImVec4(0.07f, 0.10f, 0.14f, 0.60f);

	// --- プロット・ドラッグ・ドロップ ---
	c[ImGuiCol_PlotLines] = ImVec4(0.20f, 0.78f, 1.00f, 1.00f);
	c[ImGuiCol_PlotLinesHovered] = ImVec4(0.50f, 0.90f, 1.00f, 1.00f);
	c[ImGuiCol_PlotHistogram] = ImVec4(0.10f, 0.72f, 0.95f, 1.00f);
	c[ImGuiCol_PlotHistogramHovered] = ImVec4(0.35f, 0.85f, 1.00f, 1.00f);
	c[ImGuiCol_DragDropTarget] = ImVec4(0.20f, 0.85f, 1.00f, 1.00f);

	// --- ナビゲーション・モーダル背景 ---
	c[ImGuiCol_NavHighlight] = ImVec4(0.20f, 0.78f, 1.00f, 1.00f);
	c[ImGuiCol_NavWindowingHighlight] = ImVec4(0.20f, 0.78f, 1.00f, 0.70f);
	c[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.03f, 0.06f, 0.60f);
	c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.02f, 0.05f, 0.82f);

	// --- テーマ全体の丸み・間隔調整 ---
	style.WindowRounding = 2.0f;
	style.ChildRounding = 2.0f;
	style.FrameRounding = 2.0f;
	style.PopupRounding = 2.0f;
	style.ScrollbarRounding = 3.0f;
	style.GrabRounding = 2.0f;
	style.TabRounding = 2.0f;

	// 余白を少しコンパクトに
	style.WindowPadding = ImVec2(6.0f, 5.0f);
	style.FramePadding = ImVec2(5.0f, 2.0f);
	style.ItemSpacing = ImVec2(5.0f, 3.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 2.0f);

	// インデント・スクロールバー
	style.IndentSpacing = 14.0f;
	style.ScrollbarSize = 10.0f;
	style.GrabMinSize = 8.0f;

	// 境界線
	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.TabBorderSize = 0.0f;

#endif
}

void ImGuiManager::SaveScenesJson()
{
	if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("TITLE")) {
		Editor::GetInstance()->SaveSceneJson("resources/json/titleScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEPLAY")) {
		Editor::GetInstance()->SaveSceneJson("resources/json/playScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEOVER")) {
		Editor::GetInstance()->SaveSceneJson("resources/json/gameOverScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("CLAER")) {
		Editor::GetInstance()->SaveSceneJson("resources/json/clearScene.json");
	}
}

void ImGuiManager::LoadScenesJson() {
	if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("TITLE")) {
		Editor::GetInstance()->LoadSceneJson("resources/json/titleScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEPLAY")) {
		Editor::GetInstance()->LoadSceneJson("resources/json/playScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEOVER")) {
		Editor::GetInstance()->LoadSceneJson("resources/json/gameOverScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("CLAER")) {
		Editor::GetInstance()->LoadSceneJson("resources/json/clearScene.json");
	}
}

json ImGuiManager::SaveEditorJson() const
{
	return nlohmann::json{
			{"showStats", windowState_.showStats},
			{"showCamera", windowState_.showCamera},
			{"showSound", windowState_.showSound},
	};
}

void ImGuiManager::LoadEditorJson(const nlohmann::json& j)
{
	if (j.contains("showStats")) {
		windowState_.showStats = j["showStats"].get<bool>();
	}

	if (j.contains("showCamera")) {
		windowState_.showCamera = j["showCamera"].get<bool>();
	}

	if (j.contains("showSound")) {
		windowState_.showSound = j["showSound"].get<bool>();
	}

}
