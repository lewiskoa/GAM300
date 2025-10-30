#pragma once
#include "Context/Context.h"

struct InspectorWindow : IWidget {
    BOOM_INLINE InspectorWindow(AppInterface* c) : IWidget{ c } {}

	BOOM_INLINE void OnShow() override {
        if (!m_ShowInspector) return;

        ImGui::Begin("Inspector", &m_ShowInspector);

        if (context->SelectedEntity() != entt::null) {
            Boom::Entity selectedEntity{ &context->GetEntityRegistry(), context->SelectedEntity() };

            // ==================== ENTITY NAME ====================
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

            if (selectedEntity.Has<Boom::InfoComponent>()) {
                auto& info = selectedEntity.Get<Boom::InfoComponent>();

                ImGui::Text("Entity");
                ImGui::SameLine();

                ImGui::PushItemWidth(-1);
                char buffer[256];
                strncpy_s(buffer, sizeof(buffer), info.name.c_str(), sizeof(buffer) - 1);
                if (ImGui::InputText("##EntityName", buffer, sizeof(buffer))) {
                    info.name = std::string(buffer);
                }
                ImGui::PopItemWidth();
            }

            ImGui::PopStyleVar();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // ==================== COMPONENTS ====================

            // Transform Component (can't remove)
            if (selectedEntity.Has<Boom::TransformComponent>()) {
                auto& tc = selectedEntity.Get<Boom::TransformComponent>();
                DrawComponentSection("Transform", &tc, Boom::GetTransformComponentProperties, false, nullptr);
            }

            // Camera Component
            if (selectedEntity.Has<Boom::CameraComponent>()) {
                auto& cc = selectedEntity.Get<Boom::CameraComponent>();
                DrawComponentSection("Camera", &cc, Boom::GetCameraComponentProperties, true,
                    [&]() { context->GetEntityRegistry().remove<Boom::CameraComponent>(context->SelectedEntity()); }
                );
            }

            // Model Component
            if (selectedEntity.Has<Boom::ModelComponent>()) {
                auto& mc = selectedEntity.Get<Boom::ModelComponent>();
                DrawComponentSection("Model Renderer", &mc, Boom::GetModelComponentProperties, true,
                    [&]() { context->GetEntityRegistry().remove<Boom::ModelComponent>(context->SelectedEntity()); }
                );
            }

            // RigidBody Component
            if (selectedEntity.Has<Boom::RigidBodyComponent>()) {
                auto& rc = selectedEntity.Get<Boom::RigidBodyComponent>();
                DrawComponentSection("Rigidbody", &rc, Boom::GetRigidBodyComponentProperties, true,
                    [&]() { context->GetEntityRegistry().remove<Boom::RigidBodyComponent>(context->SelectedEntity()); }
                );
            }

            // Collider Component
            if (selectedEntity.Has<Boom::ColliderComponent>()) {
                auto& col = selectedEntity.Get<Boom::ColliderComponent>();
                DrawComponentSection("Collider", &col, Boom::GetColliderComponentProperties, true,
                    [&]() { context->GetEntityRegistry().remove<Boom::ColliderComponent>(context->SelectedEntity()); }
                );
            }

            //// Animator Component
            //if (selectedEntity.Has<Boom::AnimatorComponent>()) {
            //    auto& anim = selectedEntity.Get<Boom::AnimatorComponent>();
            //    DrawComponentSection("Animator", &anim, Boom::GetAnimatorComponentProperties, true,
            //        [&]() { context->GetEntityRegistry().remove<Boom::AnimatorComponent>(context->SelectedEntity()); }
            //    );
            //}

            // Directional Light
            if (selectedEntity.Has<Boom::DirectLightComponent>()) {
                auto& dl = selectedEntity.Get<Boom::DirectLightComponent>();
                DrawComponentSection("Directional Light", &dl, Boom::GetDirectLightComponentProperties, true,
                    [&]() { context->GetEntityRegistry().remove<Boom::DirectLightComponent>(context->SelectedEntity()); }
                );
            }

            // Point Light
            if (selectedEntity.Has<Boom::PointLightComponent>()) {
                auto& pl = selectedEntity.Get<Boom::PointLightComponent>();
                DrawComponentSection("Point Light", &pl, Boom::GetPointLightComponentProperties, true,
                    [&]() { context->GetEntityRegistry().remove<Boom::PointLightComponent>(context->SelectedEntity()); }
                );
            }

            // Spot Light
            if (selectedEntity.Has<Boom::SpotLightComponent>()) {
                auto& sl = selectedEntity.Get<Boom::SpotLightComponent>();
                DrawComponentSection("Spot Light", &sl, Boom::GetSpotLightComponentProperties, true,
                    [&]() { context->GetEntityRegistry().remove<Boom::SpotLightComponent>(context->SelectedEntity()); }
                );
            }

            // Skybox Component
            if (selectedEntity.Has<Boom::SkyboxComponent>()) {
                auto& sky = selectedEntity.Get<Boom::SkyboxComponent>();
                DrawComponentSection("Skybox", &sky, Boom::GetSkyboxComponentProperties, true,
                    [&]() { context->GetEntityRegistry().remove<Boom::SkyboxComponent>(context->SelectedEntity()); }
                );
            }

            // ==================== ADD COMPONENT ====================
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Add Component", ImVec2(-1, 30))) {
                ImGui::OpenPopup("AddComponentPopup");
            }

            //if (ImGui::BeginPopup("AddComponentPopup")) {
            //    ImGui::Text("Add Component");
            //    ImGui::Separator();

            //    if (!selectedEntity.Has<Boom::CameraComponent>() && ImGui::Selectable("Camera")) {
            //        selectedEntity.Add<Boom::CameraComponent>();
            //    }
            //    if (!selectedEntity.Has<Boom::ModelComponent>() && ImGui::Selectable("Model Renderer")) {
            //        selectedEntity.Add<Boom::ModelComponent>();
            //    }
            //    if (!selectedEntity.Has<Boom::RigidBodyComponent>() && ImGui::Selectable("Rigidbody")) {
            //        selectedEntity.Add<Boom::RigidBodyComponent>();
            //    }
            //    if (!selectedEntity.Has<Boom::ColliderComponent>() && ImGui::Selectable("Collider")) {
            //        selectedEntity.Add<Boom::ColliderComponent>();
            //    }

            //    ImGui::EndPopup();
            //}
        }
        else {
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.5f - 20);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::TextWrapped("Select an entity in the hierarchy to view its properties");
            ImGui::PopStyleColor();
        }

        ImGui::End();
	}
private: //helpers
    BOOM_INLINE void DrawPropertiesUI(const xproperty::type::object* pObj, void* pInstance)
    {
        xproperty::settings::context ctx;

        for (auto& member : pObj->m_Members) {
            DrawPropertyMember(member, pInstance, ctx);
        }
    }

    BOOM_INLINE void DrawPropertyMember(const xproperty::type::members& member, void* pInstance, xproperty::settings::context& ctx)
    {
        ImGui::PushID(member.m_pName);

        if (std::holds_alternative<xproperty::type::members::var>(member.m_Variant)) {
            auto& var = std::get<xproperty::type::members::var>(member.m_Variant);

            xproperty::any value;
            var.m_pRead(pInstance, value, var.m_UnregisteredEnumSpan, ctx);

            auto typeGUID = value.getTypeGuid();
            bool changed = false;

            void* pData = &value.m_Data;

            // Label column (Unity-style: label on left, control on right)
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%s", member.m_pName);
            ImGui::SameLine(150); // Fixed label width like Unity
            ImGui::SetNextItemWidth(-1); // Fill remaining width

            if (typeGUID == xproperty::settings::var_type<float>::guid_v) {
                float* pValue = reinterpret_cast<float*>(pData);
                changed = ImGui::DragFloat("##value", pValue, 0.01f);
            }
            else if (typeGUID == xproperty::settings::var_type<glm::vec3>::guid_v) {
                glm::vec3* pValue = reinterpret_cast<glm::vec3*>(pData);
                changed = ImGui::DragFloat3("##value", &pValue->x, 0.01f);
            }
            else if (typeGUID == xproperty::settings::var_type<int32_t>::guid_v) {
                int32_t* pValue = reinterpret_cast<int32_t*>(pData);
                changed = ImGui::DragInt("##value", pValue);
            }
            else if (typeGUID == xproperty::settings::var_type<uint64_t>::guid_v) {
                uint64_t* pValue = reinterpret_cast<uint64_t*>(pData);
                changed = ImGui::InputScalar("##value", ImGuiDataType_U64, pValue);
            }
            else if (typeGUID == xproperty::settings::var_type<std::string>::guid_v) {
                std::string* pValue = reinterpret_cast<std::string*>(pData);
                char buffer[256];
                strncpy_s(buffer, pValue->c_str(), sizeof(buffer));
                if (ImGui::InputText("##value", buffer, sizeof(buffer))) {
                    *pValue = std::string(buffer);
                    changed = true;
                }
            }
            else if (value.isEnum()) {
                const auto& enumSpan = value.getEnumSpan();
                const char* currentName = value.getEnumString();

                if (ImGui::BeginCombo("##value", currentName)) {
                    for (const auto& enumItem : enumSpan) {
                        bool selected = (enumItem.m_Value == value.getEnumValue());
                        if (ImGui::Selectable(enumItem.m_pName, selected)) {
                            xproperty::any newValue;
                            newValue.set<std::string>(enumItem.m_pName);
                            var.m_pWrite(pInstance, newValue, var.m_UnregisteredEnumSpan, ctx);
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            else {
                ImGui::TextDisabled("<unsupported>");
            }

            if (changed && !member.m_bConst && var.m_pWrite) {
                var.m_pWrite(pInstance, value, var.m_UnregisteredEnumSpan, ctx);
            }
        }
        else if (std::holds_alternative<xproperty::type::members::props>(member.m_Variant)) {
            auto& props = std::get<xproperty::type::members::props>(member.m_Variant);
            auto [pChild, pChildObj] = props.m_pCast(pInstance, ctx);

            if (pChild && pChildObj) {
                // Nested object with subtle indentation
                if (ImGui::TreeNodeEx(member.m_pName, ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Indent(8.0f);
                    for (auto& childMember : pChildObj->m_Members) {
                        DrawPropertyMember(childMember, pChild, ctx);
                    }
                    ImGui::Unindent(8.0f);
                    ImGui::TreePop();
                }
            }
        }

        ImGui::PopID();
    }

    void DrawComponentSection(
        const char* componentName,
        void* pComponent,
        const xproperty::type::object* (*getPropsFunc)(void*),
        bool canRemove,
        std::function<void()> removeFunc
    )
    {
        ImGui::PushID(componentName);

        bool isOpen = ImGui::CollapsingHeader(
            componentName,
            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap
        );

        // Context menu
        if (canRemove && ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Remove Component")) {
                if (removeFunc) removeFunc();
                ImGui::EndPopup();
                ImGui::PopID();
                return;
            }
            ImGui::EndPopup();
        }

        // Settings button
        if (canRemove) {
            ImGui::SameLine(ImGui::GetWindowWidth() - 30);
            if (ImGui::SmallButton("...")) {
                ImGui::OpenPopup("ComponentSettings");
            }

            if (ImGui::BeginPopup("ComponentSettings")) {
                if (ImGui::MenuItem("Remove Component")) {
                    if (removeFunc) removeFunc();
                    ImGui::EndPopup();
                    ImGui::PopID();
                    return;
                }
                ImGui::EndPopup();
            }
        }

        if (isOpen) {
            ImGui::Indent(12.0f);
            ImGui::Spacing();

            auto* props = getPropsFunc(pComponent);
            if (props) {
                DrawPropertiesUI(props, pComponent);
            }
            else {
                ImGui::TextDisabled("No properties available");
            }

            ImGui::Spacing();
            ImGui::Unindent(12.0f);
        }

        ImGui::PopID();
        ImGui::Spacing();
    }

public:
    bool m_ShowInspector{ true };
};
