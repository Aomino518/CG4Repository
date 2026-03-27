#include "ImGuiManager.h"
#include "ImGuiUtils.h"
#include "Logger.h"
#include <Psapi.h>
#include "Entity3D.h"
#include "Sprite.h"
#include "ParticleEmitter.h"
#include "Application.h"
#include "Graphics.h"

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
	bool isLighting = model->GetIsLighting();

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

		ImGui::Checkbox("Lighting", &isLighting);

		ImGui::PopID();
	}

	model->SetIsLighting(isLighting);

#endif
}

void ImGuiManager::Stats()
{
#ifdef USE_IMGUI
	if (windowState_.showStats) {
		ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
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
		if (ImGui::BeginMenu("Window")) {
			ImGui::MenuItem("Stats", nullptr, &windowState_.showStats);
			ImGui::MenuItem("Camera", nullptr, &windowState_.showCamera);
			ImGui::MenuItem("Light", nullptr, &windowState_.showLight);
			ImGui::MenuItem("Sound", nullptr, &windowState_.showSound);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
#endif
}

void ImGuiManager::DrawParticleInspector(const std::string& name, ParticleEmitter* emitter)
{
#ifdef USE_IMGUI
	if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

		ImGui::PushID(name.c_str());

		bool changed = false;
		bool fieldChanged = false;
		ParticleManager* particleManager = ParticleManager::GetInstance();
		ParticleGroup& group = particleManager->GetGroup(name);
		BlendMode blendMode = group.blendMode_;
		AccelerationField field = particleManager->GetField(name);
		bool useBillboard = particleManager->GetUseBillboard(name);
		int emitCount = static_cast<int>(emitter->GetCount());
		float frequency = emitter->GetFrenquency();
		Transform transform = emitter->GetTransform();
		bool useField = particleManager->GetUseField(name);
		bool loop = emitter->GetIsLoop();

		// Billboard ON/OFF
		if (ImGui::Checkbox("Use Billboard", &useBillboard)) {
			particleManager->SetUseBillboard(name, useBillboard);
		}

		if (ImGuiUtils::DrawBlendModeSelector("BlendMode", blendMode)) {
			particleManager->SetBlendMode(name, blendMode);
		}

		ImGui::Separator();
		ImGui::Text("Emitter Settings");

		changed |= ImGui::DragFloat3("Emitter Translate", (float*)&transform.translate, 0.1f, -100.0f, 100.0f);
		changed |= ImGui::DragFloat3("Emitter Rotate", (float*)&transform.rotate, 0.1f, -360.0f, 360.0f);
		changed |= ImGui::DragFloat3("Emitter Scale", (float*)&transform.scale, 0.01f, 0.0f, 10.0f);

		if (changed) {
			emitter->SetTransform(transform);
		}

		if (ImGui::DragInt("Emit Count", &emitCount, 1.0f, 1, 100)) {
			emitter->SetCount(static_cast<uint32_t>(emitCount));
		}

		if (ImGui::DragFloat("Frequency", &frequency, 0.01f, 0.01f, 5.0f)) {
			emitter->SetFrenquency(frequency);
		}

		if (ImGui::CollapsingHeader("Field Settings")) {
			if (ImGui::Checkbox("Use Field", &useField)) {
				particleManager->SetUseField(name, useField);
			}

			fieldChanged |= ImGui::DragFloat3("Field Accel", (float*)&field.acceleration, 0.1f);
			fieldChanged |= ImGui::DragFloat3("AABB Min", (float*)&field.area.min, 0.1f);
			fieldChanged |= ImGui::DragFloat3("AABB Max", (float*)&field.area.max, 0.1f);

			if (fieldChanged) {
				particleManager->SetField(name, field);
			}
		}

		if (ImGui::Button("Emit Once")) {
			emitter->EmitOnce();
		}

		if (ImGui::Checkbox("Loop Emit", &loop)) {
			if (loop) {
				emitter->StartLoop();
			} else {
				emitter->StopLoop();
			}
		}

		// 生存パーティクル数の表示
		//ImGui::Text("Alive Particles: %u", particleManager->GetAliveCount());

		ImGui::PopID();
	}
#endif
}

void ImGuiManager::DrawLightWindow()
{
#ifdef USE_IMGUI
	if (windowState_.showLight) {
		DirectionalLight dirLight = LightManager::GetInstance()->GetDirectionalLight();
		PointLight pointLight = LightManager::GetInstance()->GetPointLight();
		SpotLight spotLight = LightManager::GetInstance()->GetSpotLight();

		ImGui::Begin("Light Manager");
		if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("DirectionalLight");

			bool changed = false;
			changed |= ImGui::DragFloat3("Direction", (float*)&dirLight.direction, 0.01f, -1.0f, 1.0f, "%.2f");
			changed |= ImGui::ColorEdit4("Color", (float*)&dirLight.color);
			changed |= ImGui::DragFloat("Intensity", (float*)&dirLight.intensity, 0.01f, 0.0f, 1.0f, "%.2f");

			if (changed) {
				LightManager::GetInstance()->SetDirectionalLight(&dirLight);
			}

			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("PointLight");

			bool changed = false;
			changed |= ImGui::DragFloat3("Position", (float*)&pointLight.position, 0.01f, -100.0f, 100.0f, "%.2f");
			changed |= ImGui::ColorEdit4("Color", (float*)&pointLight.color);
			changed |= ImGui::DragFloat("Intensity", (float*)&pointLight.intensity, 0.01f, 0.0f, 1.0f, "%.2f");
			changed |= ImGui::DragFloat("Radius", (float*)&pointLight.radius, 0.01f, 0.0f, 100.0f, "%.2f");
			changed |= ImGui::DragFloat("Decay", (float*)&pointLight.decay, 0.01f, 0.0f, 1.0f, "%.2f");

			if (changed) {
				LightManager::GetInstance()->SetPointLight(&pointLight);
			}

			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Spot Light", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("SpotLight");

			bool changed = false;
			changed |= ImGui::DragFloat3("Position", (float*)&spotLight.position, 0.01f, -100.0f, 100.0f, "%.2f");
			changed |= ImGui::DragFloat3("Direction", (float*)&spotLight.direction, 0.01f, -1.0f, 1.0f, "%.2f");
			changed |= ImGui::ColorEdit4("Color", (float*)&spotLight.color);
			changed |= ImGui::DragFloat("Intensity", (float*)&spotLight.intensity, 0.01f, 0.0f, 100.0f, "%.2f");
			changed |= ImGui::DragFloat("CosFalloffStart", (float*)&spotLight.cosFalloffStart, 0.01f, -1.0f, 1.0f, "%.2f");

			if (changed) {
				LightManager::GetInstance()->SetSpotLight(&spotLight);
			}

			ImGui::PopID();
		}
		ImGui::End();
	}
#endif
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
	c[ImGuiCol_Text] = ImVec4(0.80f, 0.90f, 1.00f, 1.00f);
	c[ImGuiCol_TextDisabled] = ImVec4(0.30f, 0.45f, 0.55f, 1.00f);
	c[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.03f, 0.05f, 1.00f);
	c[ImGuiCol_ChildBg] = ImVec4(0.03f, 0.04f, 0.07f, 1.00f);
	c[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.07f, 0.10f, 0.98f);
	c[ImGuiCol_Border] = ImVec4(0.10f, 0.30f, 0.45f, 0.60f);
	c[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// --- フレーム（入力欄、スライダーなど） ---
	c[ImGuiCol_FrameBg] = ImVec4(0.03f, 0.06f, 0.10f, 1.00f);
	c[ImGuiCol_FrameBgHovered] = ImVec4(0.05f, 0.18f, 0.28f, 1.00f);
	c[ImGuiCol_FrameBgActive] = ImVec4(0.07f, 0.25f, 0.38f, 1.00f);

	// --- タイトルバー・ウィンドウ装飾 ---
	c[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.10f, 0.15f, 1.00f);
	c[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.25f, 0.35f, 1.00f);
	c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.10f, 0.15f, 0.70f);
	c[ImGuiCol_MenuBarBg] = ImVec4(0.02f, 0.08f, 0.12f, 1.00f);

	// --- ボタン ---
	c[ImGuiCol_Button] = ImVec4(0.00f, 0.40f, 0.55f, 0.90f);
	c[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.55f, 0.75f, 1.00f);
	c[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.60f, 0.90f, 1.00f);

	// --- ヘッダー（ツリーノード、リスト選択など） ---
	c[ImGuiCol_Header] = ImVec4(0.00f, 0.30f, 0.45f, 0.80f);
	c[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.40f, 0.55f, 1.00f);
	c[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.55f, 0.75f, 1.00f);

	// --- スクロールバー ---
	c[ImGuiCol_ScrollbarBg] = ImVec4(0.01f, 0.02f, 0.03f, 1.00f);
	c[ImGuiCol_ScrollbarGrab] = ImVec4(0.05f, 0.25f, 0.35f, 1.00f);
	c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.08f, 0.40f, 0.55f, 1.00f);
	c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.10f, 0.55f, 0.75f, 1.00f);

	// --- スライダー ---
	c[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.55f, 0.80f, 1.00f);
	c[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.70f, 1.00f, 1.00f);

	// --- チェックマーク ---
	c[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.75f, 1.00f, 1.00f);

	// --- セパレーター ---
	c[ImGuiCol_Separator] = ImVec4(0.10f, 0.30f, 0.45f, 0.80f);
	c[ImGuiCol_SeparatorHovered] = ImVec4(0.15f, 0.45f, 0.65f, 1.00f);
	c[ImGuiCol_SeparatorActive] = ImVec4(0.20f, 0.60f, 0.85f, 1.00f);

	// --- リサイズグリップ ---
	c[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.45f, 0.65f, 0.60f);
	c[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.60f, 0.85f, 0.80f);
	c[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 0.75f, 1.00f, 1.00f);

	// --- タブ ---
	c[ImGuiCol_Tab] = ImVec4(0.00f, 0.25f, 0.35f, 0.90f);
	c[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.40f, 0.60f, 1.00f);
	c[ImGuiCol_TabActive] = ImVec4(0.00f, 0.55f, 0.80f, 1.00f);
	c[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.15f, 0.20f, 0.80f);
	c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.00f, 0.30f, 0.45f, 0.90f);

	// --- ドッキング・テーブル ---
	c[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.20f, 0.30f, 1.00f);
	c[ImGuiCol_TableBorderStrong] = ImVec4(0.10f, 0.35f, 0.55f, 1.00f);
	c[ImGuiCol_TableBorderLight] = ImVec4(0.05f, 0.25f, 0.40f, 1.00f);
	c[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	c[ImGuiCol_TableRowBgAlt] = ImVec4(0.03f, 0.05f, 0.08f, 0.50f);

	// --- プロット・ドラッグ・ドロップ ---
	c[ImGuiCol_PlotLines] = ImVec4(0.00f, 0.70f, 1.00f, 1.00f);
	c[ImGuiCol_PlotLinesHovered] = ImVec4(0.20f, 0.90f, 1.00f, 1.00f);
	c[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.60f, 0.90f, 1.00f);
	c[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 0.80f, 1.00f, 1.00f);
	c[ImGuiCol_DragDropTarget] = ImVec4(0.00f, 0.75f, 1.00f, 0.90f);

	// --- ナビゲーション・モーダル背景 ---
	c[ImGuiCol_NavHighlight] = ImVec4(0.00f, 0.60f, 0.90f, 1.00f);
	c[ImGuiCol_NavWindowingHighlight] = ImVec4(0.00f, 0.55f, 0.80f, 0.70f);
	c[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.05f, 0.10f, 0.50f);
	c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.02f, 0.05f, 0.80f);

	// --- テーマ全体の丸み・間隔調整 ---
	style.WindowRounding = 6.0f;
	style.FrameRounding = 5.0f;
	style.GrabRounding = 4.0f;
	style.ScrollbarRounding = 8.0f;
	style.TabRounding = 5.0f;
	style.FramePadding = ImVec2(6.0f, 4.0f);
	style.ItemSpacing = ImVec2(6.0f, 6.0f);
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 0.5f;
#endif
}
