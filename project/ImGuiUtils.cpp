#include "ImGuiUtils.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif

bool ImGuiUtils::DrawTransform2D(Vector2& position, float& rotation, Vector2& scale)
{
    bool changed = false;

    if (ImGui::TreeNode("Transform2D")) {
        changed |= ImGui::DragFloat2("Position", reinterpret_cast<float*>(&position), 0.01f);
        changed |= ImGui::DragFloat("Rotation", &rotation, 0.1f);
        changed |= ImGui::DragFloat2("Scale", reinterpret_cast<float*>(&scale), 0.01f, 0.0f);
        ImGui::TreePop();
    }

    return changed;
}

bool ImGuiUtils::DrawTransform3D(Vector3& position, Vector3& rotation, Vector3& scale)
{
    bool changed = false;

    if (ImGui::TreeNode("Transform")) {
        changed |= ImGui::DragFloat3("Position", reinterpret_cast<float*>(&position), 0.01f);
        changed |= ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&rotation), 0.01f);
        changed |= ImGui::DragFloat3("Scale", reinterpret_cast<float*>(&scale), 0.01f, 0.0f);
        ImGui::TreePop();
    }

    return changed;
}

bool ImGuiUtils::DrawBlendModeSelector(const char* label, BlendMode& blendMode)
{
    static const char* blendNames[] = {
        "None",
        "Normal",
        "Add",
        "Subtract",
        "Multiply",
        "Screen"
    };

    int current = static_cast<int>(blendMode);

    if (ImGui::Combo(label, &current, blendNames, IM_ARRAYSIZE(blendNames))) {
        blendMode = static_cast<BlendMode>(current);
        return true;
    }

    return false;
}

bool ImGuiUtils::DrawColor4(const char* label, Vector4& color)
{
    return ImGui::ColorEdit4(label, reinterpret_cast<float*>(&color));
}

bool ImGuiUtils::DrawVector3(const char* label, Vector3& value)
{
    return ImGui::DragFloat3(label, reinterpret_cast<float*>(&value), 0.01f);
}
