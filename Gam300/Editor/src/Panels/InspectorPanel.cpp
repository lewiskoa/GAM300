// Panels/InspectorPanel.cpp
#include "Panels/InspectorPanel.h"
#include "Editor.h"          // for Editor::GetContext()
#include "Context/Context.h"        // for Boom::AppContext + scene access
#include "Vendors/imgui/imgui.h"

using namespace EditorUI;

Boom::AppContext* InspectorPanel::GetContext() const {
    return m_Owner ? m_Owner->GetContext() : nullptr;
}

namespace EditorUI {

    // ---- templated section drawer ----
    template<typename TComponent, typename GetPropsFn>
    void InspectorPanel::DrawComponentSection(const char* title,
        TComponent* comp,
        GetPropsFn getProps,
        bool removable,
        const std::function<void()>& onRemove)
    {
        if (!comp) return;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen
            | ImGuiTreeNodeFlags_Framed
            | ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_AllowItemOverlap;

        bool open = ImGui::TreeNodeEx((void*)comp, flags, "%s", title);

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
            if constexpr (std::is_invocable_v<GetPropsFn, TComponent&>) {
                getProps(*comp);
            }
            else if constexpr (std::is_invocable_v<GetPropsFn, void*>) {
                getProps(static_cast<void*>(comp));
            }
            else if constexpr (std::is_invocable_v<GetPropsFn, const void*>) {
                getProps(static_cast<const void*>(comp));
            }
            else {
                static_assert([] {return false; }(),
                    "getProps must be invocable with TComponent& or (const) void*");
            }
            ImGui::TreePop();
        }
    }

    void InspectorPanel::Render()
    {
        if (m_ShowInspector && !*m_ShowInspector) return;

        Boom::AppContext* ctx = GetContext();
        if (!ctx) return;

        ImGui::Begin("Inspector", m_ShowInspector);

        if (m_SelectedEntity != entt::null)
        {
            // NOTE: adjust Entity wrapper to your real type/ctor signature
            // Assuming: Entity(Boom::Scene*, entt::entity)
            Boom::Entity selected{ &ctx->scene, m_SelectedEntity };

            // ===== ENTITY NAME =====
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

            if (selected.Has<InfoComponent>()) {
                auto& info = selected.Get<InfoComponent>();
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
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            // ===== COMPONENTS =====
            if (selected.Has<TransformComponent>()) {
                auto& tc = selected.Get<TransformComponent>();
                DrawComponentSection("Transform", &tc, GetTransformComponentProperties, false, nullptr);
            }

            if (selected.Has<CameraComponent>()) {
                auto& cc = selected.Get<CameraComponent>();
                DrawComponentSection("Camera", &cc, GetCameraComponentProperties, true,
                    [&]() { ctx->scene.remove<CameraComponent>(m_SelectedEntity); });
            }

            if (selected.Has<ModelComponent>()) {
                auto& mc = selected.Get<ModelComponent>();
                DrawComponentSection("Model Renderer", &mc, GetModelComponentProperties, true,
                    [&]() { ctx->scene.remove<ModelComponent>(m_SelectedEntity); });
            }

            if (selected.Has<RigidBodyComponent>()) {
                auto& rc = selected.Get<RigidBodyComponent>();
                DrawComponentSection("Rigidbody", &rc, GetRigidBodyComponentProperties, true,
                    [&]() { ctx->scene.remove<RigidBodyComponent>(m_SelectedEntity); });
            }

            if (selected.Has<ColliderComponent>()) {
                auto& col = selected.Get<ColliderComponent>();
                DrawComponentSection("Collider", &col, GetColliderComponentProperties, true,
                    [&]() { ctx->scene.remove<ColliderComponent>(m_SelectedEntity); });
            }

            if (selected.Has<DirectLightComponent>()) {
                auto& dl = selected.Get<DirectLightComponent>();
                DrawComponentSection("Directional Light", &dl, GetDirectLightComponentProperties, true,
                    [&]() { ctx->scene.remove<DirectLightComponent>(m_SelectedEntity); });
            }

            if (selected.Has<PointLightComponent>()) {
                auto& pl = selected.Get<PointLightComponent>();
                DrawComponentSection("Point Light", &pl, GetPointLightComponentProperties, true,
                    [&]() { ctx->scene.remove<PointLightComponent>(m_SelectedEntity); });
            }

            if (selected.Has<SpotLightComponent>()) {
                auto& sl = selected.Get<SpotLightComponent>();
                DrawComponentSection("Spot Light", &sl, GetSpotLightComponentProperties, true,
                    [&]() { ctx->scene.remove<SpotLightComponent>(m_SelectedEntity); });
            }

            if (selected.Has<SkyboxComponent>()) {
                auto& sky = selected.Get<SkyboxComponent>();
                DrawComponentSection("Skybox", &sky, GetSkyboxComponentProperties, true,
                    [&]() { ctx->scene.remove<SkyboxComponent>(m_SelectedEntity); });
            }

            // ===== Add Component =====
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
            if (ImGui::Button("Add Component", ImVec2(-1, 30))) {
                ImGui::OpenPopup("AddComponentPopup");
            }
            // (AddComponentPopup contents go here)
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

} // namespace EditorUI
