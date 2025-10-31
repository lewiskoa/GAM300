#pragma once
#include "Context/Context.h"

struct InspectorWindow : IWidget {
private:
    const std::string_view CUSTOM_PAYLOAD_TYPE{ "DND_INT" };
public:
    BOOM_INLINE InspectorWindow(AppInterface* c) : IWidget{ c } {}

	BOOM_INLINE void OnShow() override {
        if (!m_ShowInspector) return;

        ImGui::Begin("Inspector", &m_ShowInspector);

        if (context->SelectedEntity() != entt::null) {
            EntityUpdate();
        }
        else if (context->SelectedAsset().id != 0u) {
            AssetUpdate();
        }
        else {
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.5f - 20);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::TextWrapped("Select an entity in the hierarchy or an asset in resources to view its properties");
            ImGui::PopStyleColor();
        }

        ImGui::End();
	}

private:
    BOOM_INLINE void EntityUpdate() {
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
        if (ImGui::CollapsingHeader("Model Renderer")) {
            //2 for now (model.fbx & material)
            //ImGui::
        }

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

    BOOM_INLINE void AssetUpdate() {
        context->ModifyAsset([&](auto* asset) {
            ImGui::Text("Modifying: %s", asset->name.c_str());
            if (asset->type == AssetType::MATERIAL) {
                MaterialAsset* mat{ dynamic_cast<MaterialAsset*>(asset) };

                //TODO: showcase material as textured sphere
                //data variables (showcase texture name instead of map id)
                // toggle between mapID and standard slider (vec3/float)

                if (ImGui::CollapsingHeader("Maps", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::BeginTable("##maps", 6, ImGuiTableFlags_SizingFixedFit);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                    InputAssetWidget("albedo map", mat->albedoMapID);
                    InputAssetWidget("normal map", mat->normalMapID);
                    InputAssetWidget("roughness map", mat->roughnessMapID);
                    InputAssetWidget("metallic map", mat->metallicMapID);
                    InputAssetWidget("occlusion map", mat->occlusionMapID);
                    InputAssetWidget("emissive map", mat->emissiveMapID);
                    ImGui::EndTable();
                }

                if (ImGui::CollapsingHeader("Variables", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::DragFloat3("albedo", &mat->data.albedo[0], 0.01f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::DragFloat3("emissive", &mat->data.emissive[0], 0.01f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::DragFloat("roughness", &mat->data.roughness, 0.01f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::DragFloat("metallic", &mat->data.metallic, 0.01f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::DragFloat("occlusion", &mat->data.occlusion, 0.01f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
                }
            }
            else if (asset->type == AssetType::TEXTURE) {
                TextureAsset* tex{ dynamic_cast<TextureAsset*>(asset) };
                ImGui::Image((ImTextureID)(*tex->data.get()), { 256, 256 });

                //data variables (these are settings for compression)
                if (ImGui::CollapsingHeader("Compression Settings:", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Checkbox("Will Compress?", &tex->data->isCompileAsCompressed);
                    if (tex->data->isCompileAsCompressed) {
                        ImGui::SliderFloat("Quality", &tex->data->quality, 0.f, 1.f);
                        ImGui::SliderInt("Alpha Threshold", &tex->data->alphaThreshold, 0, 255);
                        ImGui::SliderInt("Mip Level", &tex->data->mipLevel, 1, 24);
                        ImGui::Checkbox("Gamma", &tex->data->isGamma);
                    }
                }
            }
            });
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

    BOOM_INLINE void DrawComponentSection(
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

    BOOM_INLINE void AcceptIDDrop(AssetID& data) {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_INT"))
            {
                IM_ASSERT(payload->DataSize == sizeof(AssetID));
                data = *(AssetID const*)payload->Data;
                ImGui::Text("Dropped ID: %llu", data);
            }
            ImGui::EndDragDropTarget();
        }
    }
    BOOM_INLINE void InputAssetWidget(char const* label, AssetID& data) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(label);
        ImGui::SameLine();

        ImGui::TableSetColumnIndex(1);
        ImVec2 const fieldSize{ ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight()}; //auto height
        ImGui::PushID(label);
        if (ImGui::Button(context->GetAssetName<TextureAsset>(data).c_str(), fieldSize)) {
            //TODO: clicking button opens asset picker window
        }
        AcceptIDDrop(data);
        ImGui::PopID();
    }
public:
    bool m_ShowInspector{ true };


};
