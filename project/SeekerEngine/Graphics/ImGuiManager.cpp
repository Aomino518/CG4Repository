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

void ImGuiManager::DrawWorldFildWindow()
{
	if (!windowState_.showWorldField) {
		return;
	}

	ImGui::Begin("WorldField Manager");
	auto worldFieldMgr = WorldFieldManager::GetInstance();
	auto worldFields = worldFieldMgr->GetWorldFields();

	for (auto& [name, field] : worldFields) {

		if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::PushID(name.c_str());
			Vector3 pos = field.GetPosition();
			Vector3 accelerarion = field.GetAcceleration();
			AABB area = field.GetAABB();
			bool isActive = field.GetIsActive();
			bool changed = false;

			changed |= ImGui::DragFloat3("Position", (float*)&pos, 0.01f);
			changed |= ImGui::DragFloat3("Acceleration", (float*)&accelerarion, 0.01f);
			// minをmaxが下回ると落ちるのでのちのち修正
			changed |= ImGui::DragFloat3("AABB Min", (float*)&area.min, 0.1f);
			changed |= ImGui::DragFloat3("AABB Max", (float*)&area.max, 0.1f);
			changed |= ImGui::Checkbox("isActive", &isActive);

			if (changed) {
				field.SetPosition(pos);
				field.SetAcceleration(accelerarion);
				field.SetAABB(area);
				field.SetIsActive(isActive);
				worldFieldMgr->SetField(name, field);
			}
			ImGui::PopID();
		}
	}
	ImGui::End();
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
			ImGui::MenuItem("WorldField", nullptr, &windowState_.showWorldField);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
#endif
}

void ImGuiManager::DrawParticleInspector(const std::string& name)
{
#ifdef USE_IMGUI
	if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

		ImGui::PushID(name.c_str());

		bool changed = false;
		bool fieldChanged = false;
		ParticleManager* particleManager = ParticleManager::GetInstance();
		EmitterManager* emitterMgr = EmitterManager::GetInstance();
		ParticleGroup& group = particleManager->GetGroup(name);
		ParticleEmitter* emitter = emitterMgr->GetEmitter(name);
		BlendMode blendMode = group.blendMode_;
		bool useBillboard = particleManager->GetUseBillboard(name);
		int emitCount = static_cast<int>(emitter->GetCount());
		float frequency = emitter->GetFrenquency();
		Transform transform = emitter->GetTransform();
		ParticleConfig config = emitter->GetConfig();
		bool loop = emitter->GetIsLoop();

		// Field
		AccelerationField field = emitter->GetLocalField();
		bool isFieldActive = field.GetIsActive();
		Vector3 acceleration = field.GetAcceleration();
		AABB area = field.GetAABB();

		if (ImGuiUtils::DrawBlendModeSelector("BlendMode", blendMode)) {
			particleManager->SetBlendMode(name, blendMode);
		}

		// Billboard ON/OFF
		if (ImGui::Checkbox("Use Billboard", &useBillboard)) {
			particleManager->SetUseBillboard(name, useBillboard);
		}

		if (ImGui::Checkbox("Loop Emit", &loop)) {
			if (loop) {
				emitter->StartLoop();
			} else {
				emitter->StopLoop();
			}
		}

		if (ImGui::Button("Emit Once")) {
			emitter->EmitOnce();
		}

		ImGui::Separator();
		if (ImGui::TreeNode("Emitter Settings")) {

			changed |= ImGui::DragFloat3("Emitter Translate", &transform.translate.x, 0.1f, -100.0f, 100.0f);
			changed |= ImGui::DragFloat3("Emitter Rotate", &transform.rotate.x, 0.1f, -360.0f, 360.0f);
			changed |= ImGui::DragFloat3("Emitter Scale", &transform.scale.x, 0.01f, 0.0f, 10.0f);

			if (changed) {
				emitter->SetTransform(transform);
			}

			if (ImGui::DragInt("Emit Count", &emitCount, 1.0f, 1, 100)) {
				emitter->SetCount(static_cast<uint32_t>(emitCount));
			}

			if (ImGui::DragFloat("Frequency", &frequency, 0.01f, 0.01f, 5.0f)) {
				emitter->SetFrenquency(frequency);
			}

			ImGui::TreePop();
		}

		if (ImGuiUtils::DrawEditParticleConfig("Config", config)) {
			emitter->SetConfig(config);
		}

		if (ImGui::TreeNode("Field Settings")) {
			if (ImGui::Checkbox("Use Field", &isFieldActive)) {
				field.SetIsActive(isFieldActive);
				emitter->SetLocalField(field);
			}

			fieldChanged |= ImGui::DragFloat3("Field Accel", (float*)&acceleration, 0.1f);
			fieldChanged |= ImGui::DragFloat3("AABB Min", (float*)&area.min, 0.1f);
			fieldChanged |= ImGui::DragFloat3("AABB Max", (float*)&area.max, 0.1f);

			if (fieldChanged) {
				field.SetAcceleration(acceleration);
				field.SetAABB(area);
				emitter->SetLocalField(field);
			}
			ImGui::TreePop();
		}

		//生存パーティクル数の表示
		ImGui::Text("Alive Particles: %u", particleManager->GetkNumInstance(name));

		ImGui::PopID();
	}
#endif
}

void ImGuiManager::DrawLightWindow()
{
#ifdef USE_IMGUI
	if (!windowState_.showLight) {
		return;
	}

	LightManager* lightMgr = LightManager::GetInstance();

	DirectionalLight dirLight = lightMgr->GetDirectionalLight();
	const auto& pointLights = lightMgr->GetPointLights();
	const auto& spotLights = lightMgr->GetSpotLights();

	bool isLighting = ModelManager::GetInstance()->GetIsModelLighting();
	bool isChangedLighting = false;

	static float yaw = 0.0f;
	static float pitch = -45.0f;

	ImGui::Begin("Light Manager");

	isChangedLighting |= ImGui::Checkbox("isLighting", &isLighting);
	if (isChangedLighting) {
		ModelManager::GetInstance()->SetIsLighting(isLighting);
	}

	// Directional
	if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushID("DirectionalLight");

		bool changed = false;
		changed |= ImGui::SliderFloat("Yaw", &yaw, -180.0f, 180.0f, "%.1f deg");
		changed |= ImGui::SliderFloat("Pitch", &pitch, -89.0f, 89.0f, "%.1f deg");
		changed |= ImGui::ColorEdit4("Color", (float*)&dirLight.color);
		changed |= ImGui::DragFloat("Intensity", &dirLight.intensity, 0.01f, 0.0f, 10.0f, "%.2f");

		if (changed) {
			float yawRad = yaw * std::numbers::pi_v<float> / 180.0f;
			float pitchRad = pitch * std::numbers::pi_v<float> / 180.0f;

			dirLight.direction.x = cosf(pitchRad) * cosf(yawRad);
			dirLight.direction.y = sinf(pitchRad);
			dirLight.direction.z = cosf(pitchRad) * sinf(yawRad);

			lightMgr->SetDirectionalLight(&dirLight);
		}

		ImGui::PopID();
	}

	// Point
	if (ImGui::CollapsingHeader("Point Lights", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (const auto& [name, data] : pointLights)
		{
			PointLight pointLight = data;

			ImGui::PushID(name.c_str());
			if (ImGui::TreeNode(name.c_str()))
			{
				bool changed = false;
				changed |= ImGui::DragFloat3("Position", (float*)&pointLight.position, 0.01f, -100.0f, 100.0f, "%.2f");
				changed |= ImGui::ColorEdit4("Color", (float*)&pointLight.color);
				changed |= ImGui::DragFloat("Intensity", &pointLight.intensity, 0.01f, 0.0f, 10.0f, "%.2f");
				changed |= ImGui::DragFloat("Radius", &pointLight.radius, 0.01f, 0.0f, 100.0f, "%.2f");
				changed |= ImGui::DragFloat("Decay", &pointLight.decay, 0.01f, 0.0f, 10.0f, "%.2f");

				if (changed) {
					std::string lightName = name;
					lightMgr->SetPointLight(lightName, &pointLight);
				}

				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}

	// Spot
	if (ImGui::CollapsingHeader("Spot Lights", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (const auto& [name, data] : spotLights)
		{
			SpotLight spotLight = data;

			ImGui::PushID(name.c_str());
			if (ImGui::TreeNode(name.c_str()))
			{
				bool changed = false;
				changed |= ImGui::DragFloat3("Position", (float*)&spotLight.position, 0.01f, -100.0f, 100.0f, "%.2f");
				changed |= ImGui::ColorEdit4("Color", (float*)&spotLight.color);
				changed |= ImGui::DragFloat("Intensity", &spotLight.intensity, 0.01f, 0.0f, 100.0f, "%.2f");
				changed |= ImGui::DragFloat("Distance", &spotLight.distance, 0.01f, 0.0f, 100.0f, "%.2f");
				changed |= ImGui::DragFloat("Decay", &spotLight.decay, 0.01f, 0.0f, 10.0f, "%.2f");
				changed |= ImGui::DragFloat("CosAngle", &spotLight.cosAngle, 0.01f, -1.0f, 1.0f, "%.2f");
				changed |= ImGui::DragFloat("CosFalloffStart", &spotLight.cosFalloffStart, 0.01f, -1.0f, 1.0f, "%.2f");

				changed |= ImGui::DragFloat3("Direction", (float*)&spotLight.direction, 0.01f, -1.0f, 1.0f, "%.2f");

				if (changed) {
					spotLight.direction = Normalize(spotLight.direction);
					std::string lightName = name;
					lightMgr->SetSpotLight(lightName, &spotLight);
				}

				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}

	ImGui::End();
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
