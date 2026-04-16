#include "Editor.h"
#include "Sprite.h"
#include "Entity3D.h"
#include "EmitterManager.h"
#include "ParticleManager.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

Editor* Editor::GetInstance()
{
    static Editor instance;
    return &instance;
}

void Editor::Draw()
{
#ifdef USE_IMGUI
    DrawHierarchy();
    DrawInspector();
#endif
}

void Editor::RegisterSprite(const std::string& name, Sprite* sprite)
{
	sprites_[name] = sprite;
}

void Editor::RegisterModel(const std::string& name, Entity3D* model)
{
	models_[name] = model;
}

void Editor::RegisterParticle(const std::string& name)
{
	particles_[name];
}

void Editor::Clear()
{
    sprites_.clear();
    models_.clear();
    particles_.clear();
    selection_ = {};
}

void Editor::DrawHierarchy()
{
    ImGui::Begin("Hierarchy");

    // =========================
    // Sprite
    // =========================
    if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (auto& [name, sprite] : sprites_) {
            bool selected = (selection_.category == InspectorCategory::Sprite && selection_.name == name);
            if (ImGui::Selectable(name.c_str(), selected)) {
                selection_.category = InspectorCategory::Sprite;
                selection_.name = name;
            }
        }
    }

    // =========================
    // Model
    // =========================
    if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (auto& [name, model] : models_) {
            bool selected = (selection_.category == InspectorCategory::Model && selection_.name == name);
            if (ImGui::Selectable(name.c_str(), selected)) {
                selection_.category = InspectorCategory::Model;
                selection_.name = name;
            }
        }
    }

    // =========================
    // Particle
    // =========================
    if (ImGui::CollapsingHeader("Particle", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& emitters = EmitterManager::GetInstance()->GetEmittersAndNames();

        for (auto& [name, emitter] : emitters) {

            bool selected = (selection_.category == InspectorCategory::Particle && selection_.name == name);

            if (ImGui::Selectable(name.c_str(), selected)) {
                selection_.category = InspectorCategory::Particle;
                selection_.name = name;
            }
        }
    }

    ImGui::End();
}

void Editor::DrawInspector()
{
    ImGui::Begin("Inspector");

    switch (selection_.category) {

    case InspectorCategory::Sprite:
    {
        auto it = sprites_.find(selection_.name);
        if (it != sprites_.end()) {
            ImGui::Text("Name: %s", it->first.c_str());
            it->second->DrawImGui();
        }
        break;
    }

    case InspectorCategory::Model:
    {
        auto it = models_.find(selection_.name);
        if (it != models_.end()) {
            ImGui::Text("Name: %s", it->first.c_str());
            it->second->DrawImGui();
        }
        break;
    }

    case InspectorCategory::Particle:
    {
        auto* emitter = EmitterManager::GetInstance()->GetEmitter(selection_.name);

        if (emitter) {
            emitter->DrawImGui();
        }

        ParticleManager::GetInstance()->DrawParticleGroupImGui(selection_.name);
        break;
    }

    default:
        ImGui::Text("Nothing selected.");
        break;
    }

    ImGui::End();
}
