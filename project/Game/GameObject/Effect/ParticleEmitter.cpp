#include "ParticleEmitter.h"
#include "DebugDraw.h"

ParticleEmitter::ParticleEmitter(const std::string& groupName, 
	const ParticleConfig& config,
	uint32_t count,
	float frequency)
	: groupName_(groupName),
	config_(config),
	count_(count),
	frequency_(frequency),
	frequencyTime_(0.0f)
{
	transform_.translate = { 0.0f, 0.0f, 0.0f };
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	transform_.scale = { 1.0f, 1.0f, 1.0f };
}

void ParticleEmitter::EmitOnce()
{
	ParticleManager::GetInstance()->Emit(
		groupName_, 
		config_,
		transform_.translate,
		count_);
}

void ParticleEmitter::StartLoop()
{
	isLoop_ = true;
	frequencyTime_ = 0.0f;
}

void ParticleEmitter::StopLoop()
{
	isLoop_ = false;
}

void ParticleEmitter::Update()
{
	if (isLoop_) {
		frequencyTime_ += kDeltaTime;

		while (frequency_ <= frequencyTime_) {
			ParticleManager::GetInstance()->Emit(
				groupName_, 
				config_,
				transform_.translate,
				count_);
			frequencyTime_ -= frequency_;
		}
	}
}

void ParticleEmitter::SetSpawnShapeBox(const Vector3& min, const Vector3& max)
{
	config_.shape = SpawnShape::Box;
	config_.boxMin = min;
	config_.boxMax = max;
}

void ParticleEmitter::SetSpawnShapeSphere(float radius)
{
	config_.shape = SpawnShape::Sphere;
	config_.sphereRadius = radius;
}

void ParticleEmitter::DrawDebug() {
#ifdef _DEBUG
	DebugDraw::DrawAABB(transform_.translate, localField_.GetAABB(), Vector4(0.2f, 0.6f, 1.0f, 1.0f), DebugDrawMode::Wireframe);
#endif
}

void ParticleEmitter::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Text("Emitter: %s", groupName_.c_str());

    // =========================
    // Transform
    // =========================
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("Position", &transform_.translate.x, 0.1f);
        ImGui::DragFloat3("Rotation", &transform_.rotate.x, 0.1f);
        ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f);
    }

    // =========================
    // Emit Control
    // =========================
    if (ImGui::CollapsingHeader("Emit Control", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::DragInt("Count", reinterpret_cast<int*>(&count_), 1, 1, 1000);
        ImGui::DragFloat("Frequency", &frequency_, 0.01f, 0.01f, 10.0f);

        if (ImGui::Button("Emit Once")) {
            EmitOnce();
        }

        if (ImGui::Button(isLoop_ ? "Stop Loop" : "Start Loop")) {
            if (isLoop_) StopLoop();
            else StartLoop();
        }
    }

    // =========================
    // Spawn Shape
    // =========================
    if (ImGui::CollapsingHeader("Spawn Shape", ImGuiTreeNodeFlags_DefaultOpen)) {

        int shape = static_cast<int>(config_.shape);
        const char* shapes[] = { "Box", "Sphere" };
        if (ImGui::Combo("Shape", &shape, shapes, IM_ARRAYSIZE(shapes))) {
            config_.shape = static_cast<SpawnShape>(shape);
        }

        if (config_.shape == SpawnShape::Box) {
            ImGui::DragFloat3("Box Min", &config_.boxMin.x, 0.1f);
            ImGui::DragFloat3("Box Max", &config_.boxMax.x, 0.1f);
        } else {
            ImGui::DragFloat("Radius", &config_.sphereRadius, 0.01f, 0.01f, 100.0f);
        }
    }

    // =========================
    // Velocity / Rotation
    // =========================
    if (ImGui::CollapsingHeader("Velocity / Rotation", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("Min Velocity", &config_.minVelocity.x, 0.1f);
        ImGui::DragFloat3("Max Velocity", &config_.maxVelocity.x, 0.1f);

        ImGui::DragFloat3("Min Rotate", &config_.minRotate.x, 0.1f);
        ImGui::DragFloat3("Max Rotate", &config_.maxRotate.x, 0.1f);

        ImGui::DragFloat3("Min RotVel", &config_.minRotateVelocity.x, 0.1f);
        ImGui::DragFloat3("Max RotVel", &config_.maxRotateVelocity.x, 0.1f);
    }

    // =========================
    // Color
    // =========================
    if (ImGui::CollapsingHeader("Color", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::ColorEdit4("Start Color Min", &config_.startColorMin.x);
        ImGui::ColorEdit4("Start Color Max", &config_.startColorMax.x);
        ImGui::ColorEdit4("End Color Min", &config_.endColorMin.x);
        ImGui::ColorEdit4("End Color Max", &config_.endColorMax.x);
    }

    // =========================
    // Scale
    // =========================
    if (ImGui::CollapsingHeader("Scale", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("Start Scale Min", &config_.startScaleMin.x, 0.1f);
        ImGui::DragFloat3("Start Scale Max", &config_.startScaleMax.x, 0.1f);
        ImGui::DragFloat3("End Scale Min", &config_.endScaleMin.x, 0.1f);
        ImGui::DragFloat3("End Scale Max", &config_.endScaleMax.x, 0.1f);
    }

    // =========================
    // Lifetime
    // =========================
    if (ImGui::CollapsingHeader("Lifetime", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Min Life", &config_.minLifeTime, 0.1f, 0.01f);
        ImGui::DragFloat("Max Life", &config_.maxLifeTime, 0.1f, 0.01f);
    }

#endif
}