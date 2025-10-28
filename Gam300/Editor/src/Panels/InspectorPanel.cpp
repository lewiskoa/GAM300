#include "Panels/InspectorPanel.h"

// ImGui
#include "Vendors/imgui/imgui.h"
#include "BoomEngine.h"

// Engine / editor types (adjust paths to your project)
#include "Context/Context.h"
#include "Context/DebugHelpers.h"

template<typename TComponent, typename GetPropsFn>
void InspectorPanel::DrawComponentSection(const char* title,
    TComponent* comp,
    GetPropsFn getProps,
    bool removable,
    const std::function<void()>& onRemove)
{
    if (!comp) return;

    // Collapsing header with a simple “...” context menu for removal
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed
        | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap;

    bool open = ImGui::TreeNodeEx((void*)comp, flags, "%s", title);

    // Right-aligned context menu button
    float lineHeight = ImGui::GetTextLineHeight();
    float availX = ImGui::GetContentRegionAvail().x;
    ImGui::SameLine(availX - lineHeight * 1.25f);
    if (removable) {
        if (ImGui::Button("...", ImVec2(lineHeight, lineHeight))) {
            ImGui::OpenPopup("ComponentSettings");
        }
        if (ImGui::BeginPopup("ComponentSettings")) {
            if (ImGui::MenuItem("Remove Component")) {
                if (onRemove) onRemove();
                ImGui::EndPopup();
                if (open) ImGui::TreePop();
                return;
            }
            ImGui::EndPopup();
        }
    }

    if (open) {
        // Delegate to your property UI function
        getProps(*comp);
        ImGui::TreePop();
    }
}

void InspectorPanel::Render()
{
    if (m_ShowInspector && !*m_ShowInspector)
        return;

    if (!m_Context)
        return;

    ImGui::Begin("Inspector", m_ShowInspector);

    if (m_SelectedEntity && *m_SelectedEntity != entt::null)
    {
        Boom::Entity selected{ &m_Context->scene, *m_SelectedEntity };

        // ==================== ENTITY NAME ====================
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

        if (selected.Has<Boom::InfoComponent>()) {
            auto& info = selected.Get<Boom::InfoComponent>();

            ImGui::TextUnformatted("Entity");
            ImGui::SameLine();

            ImGui::PushItemWidth(-1);
#ifdef _MSC_VER
            strncpy_s(m_NameBuffer, sizeof(m_NameBuffer), info.name.c_str(), sizeof(m_NameBuffer) - 1);
#else
            std::snprintf(m_NameBuffer, sizeof(m_NameBuffer), "%s", info.name.c_str());
#endif
            if (ImGui::InputText("##EntityName", m_NameBuffer, sizeof(m_NameBuffer))) {
                info.name = std::string(m_NameBuffer);
            }
            ImGui::PopItemWidth();
        }

        ImGui::PopStyleVar();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ==================== COMPONENTS ====================

        // Transform (non-removable)
        if (selected.Has<Boom::TransformComponent>()) {
            auto& tc = selected.Get<Boom::TransformComponent>();
            DrawComponentSection("Transform", &tc, Boom::GetTransformComponentProperties, false, nullptr);
        }

        // Camera
        if (selected.Has<Boom::CameraComponent>()) {
            auto& cc = selected.Get<Boom::CameraComponent>();
            DrawComponentSection("Camera", &cc, Boom::GetCameraComponentProperties, true,
                [&]() { m_Context->scene.remove<Boom::CameraComponent>(*m_SelectedEntity); });
        }

        // Model Renderer
        if (selected.Has<Boom::ModelComponent>()) {
            auto& mc = selected.Get<Boom::ModelComponent>();
            DrawComponentSection("Model Renderer", &mc, Boom::GetModelComponentProperties, true,
                [&]() { m_Context->scene.remove<Boom::ModelComponent>(*m_SelectedEntity); });
        }

        // Rigidbody
        if (selected.Has<Boom::RigidBodyComponent>()) {
            auto& rc = selected.Get<Boom::RigidBodyComponent>();
            DrawComponentSection("Rigidbody", &rc, Boom::GetRigidBodyComponentProperties, true,
                [&]() { m_Context->scene.remove<Boom::RigidBodyComponent>(*m_SelectedEntity); });
        }

        // Collider
        if (selected.Has<Boom::ColliderComponent>()) {
            auto& col = selected.Get<Boom::ColliderComponent>();
            DrawComponentSection("Collider", &col, Boom::GetColliderComponentProperties, true,
                [&]() { m_Context->scene.remove<Boom::ColliderComponent>(*m_SelectedEntity); });
        }

        // Directional Light
        if (selected.Has<Boom::DirectLightComponent>()) {
            auto& dl = selected.Get<Boom::DirectLightComponent>();
            DrawComponentSection("Directional Light", &dl, Boom::GetDirectLightComponentProperties, true,
                [&]() { m_Context->scene.remove<Boom::DirectLightComponent>(*m_SelectedEntity); });
        }

        // Point Light
        if (selected.Has<Boom::PointLightComponent>()) {
            auto& pl = selected.Get<Boom::PointLightComponent>();
            DrawComponentSection("Point Light", &pl, Boom::GetPointLightComponentProperties, true,
                [&]() { m_Context->scene.remove<Boom::PointLightComponent>(*m_SelectedEntity); });
        }

        // Spot Light
        if (selected.Has<Boom::SpotLightComponent>()) {
            auto& sl = selected.Get<Boom::SpotLightComponent>();
            DrawComponentSection("Spot Light", &sl, Boom::GetSpotLightComponentProperties, true,
                [&]() { m_Context->scene.remove<Boom::SpotLightComponent>(*m_SelectedEntity); });
        }

        // Skybox
        if (selected.Has<Boom::SkyboxComponent>()) {
            auto& sky = selected.Get<Boom::SkyboxComponent>();
            DrawComponentSection("Skybox", &sky, Boom::GetSkyboxComponentProperties, true,
                [&]() { m_Context->scene.remove<Boom::SkyboxComponent>(*m_SelectedEntity); });
        }

        // ==================== ADD COMPONENT (UI only stub) ====================
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Add Component", ImVec2(-1, 30))) {
            ImGui::OpenPopup("AddComponentPopup");
        }

        // You can re-enable your popup when ready
        // if (ImGui::BeginPopup("AddComponentPopup")) { ... ImGui::EndPopup(); }

    }
    else
    {
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.5f - 20.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::TextWrapped("Select an entity in the hierarchy to view its properties");
        ImGui::PopStyleColor();
    }

    ImGui::End();
}
