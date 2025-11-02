// Panels/InspectorPanel.cpp
#include "Panels/InspectorPanel.h"
#include "Editor.h"          // for Editor::GetContext()
#include "Context/Context.h"        // for Boom::AppContext + scene access
#include "Vendors/imgui/imgui.h"
#include "Auxiliaries/Assets.h"
#include "Context/DebugHelpers.h"

using namespace EditorUI;

Boom::AppContext* InspectorPanel::GetContext() const {
    return m_Owner ? m_Owner->GetContext() : nullptr;
}

namespace EditorUI {

    InspectorPanel::InspectorPanel(Editor* owner, bool* showFlag)
        : m_Owner(owner)
        , m_ShowInspector(showFlag)
        , m_App(dynamic_cast<Boom::AppInterface*>(owner))
    {
        DEBUG_POINTER(m_App, "AppInterface");
    }

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

        DeleteUpdate();
        if (m_App->SelectedEntity() != entt::null) {
            EntityUpdate();
        }
        else if (m_App->SelectedAsset().id != 0u) {
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

    void InspectorPanel::EntityUpdate() {
        Boom::AppContext* ctx = GetContext();
        // NOTE: adjust Entity wrapper to your real type/ctor signature
            // Assuming: Entity(Boom::Scene*, entt::entity)
        Boom::Entity selected{ &ctx->scene, m_App->SelectedEntity()};

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
                [&]() { ctx->scene.remove<CameraComponent>(m_App->SelectedEntity()); });
        }

        // Model Component
        if (selected.Has<Boom::ModelComponent>()) {
            auto& mc = selected.Get<Boom::ModelComponent>();

            if (ImGui::CollapsingHeader("Model Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
                //2 for now (model.fbx & material)
                ImGui::BeginTable("##maps", 6, ImGuiTableFlags_SizingFixedFit);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                InputAssetWidget<CONSTANTS::DND_PAYLOAD_MODEL>("Model", mc.modelID);
                InputAssetWidget<CONSTANTS::DND_PAYLOAD_MATERIAL>("Material", mc.materialID);
                ImGui::EndTable();
            }
        }

        if (selected.Has<RigidBodyComponent>()) {
            auto& rc = selected.Get<RigidBodyComponent>();
            DrawComponentSection("Rigidbody", &rc, GetRigidBodyComponentProperties, true,
                [&]() { ctx->scene.remove<RigidBodyComponent>(m_App->SelectedEntity()); });
        }

        if (selected.Has<ColliderComponent>()) {
            auto& col = selected.Get<ColliderComponent>();
            DrawComponentSection("Collider", &col, GetColliderComponentProperties, true,
                [&]() { ctx->scene.remove<ColliderComponent>(m_App->SelectedEntity()); });
        }

        if (selected.Has<DirectLightComponent>()) {
            auto& dl = selected.Get<DirectLightComponent>();
            DrawComponentSection("Directional Light", &dl, GetDirectLightComponentProperties, true,
                [&]() { ctx->scene.remove<DirectLightComponent>(m_App->SelectedEntity()); });
        }

        if (selected.Has<PointLightComponent>()) {
            auto& pl = selected.Get<PointLightComponent>();
            DrawComponentSection("Point Light", &pl, GetPointLightComponentProperties, true,
                [&]() { ctx->scene.remove<PointLightComponent>(m_App->SelectedEntity()); });
        }

        if (selected.Has<SpotLightComponent>()) {
            auto& sl = selected.Get<SpotLightComponent>();
            DrawComponentSection("Spot Light", &sl, GetSpotLightComponentProperties, true,
                [&]() { ctx->scene.remove<SpotLightComponent>(m_App->SelectedEntity()); });
        }

        if (selected.Has<SkyboxComponent>()) {
            auto& sky = selected.Get<SkyboxComponent>();
            DrawComponentSection("Skybox", &sky, GetSkyboxComponentProperties, true,
                [&]() { ctx->scene.remove<SkyboxComponent>(m_App->SelectedEntity()); });
        }

        // ===== Add Component =====
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
        if (ImGui::Button("Add Component", ImVec2(-1, 30))) {
            ImGui::OpenPopup("AddComponentPopup");
        }
        // (AddComponentPopup contents go here)
    }

    void InspectorPanel::AssetUpdate() {
        m_App->ModifyAsset([&](auto* asset) {
            ImGui::Text("Modifying: %s (%d)", asset->name.c_str(), asset->uid);
            if (asset->type == AssetType::MATERIAL) {
                MaterialAsset* mat{ dynamic_cast<MaterialAsset*>(asset) };

                //TODO: showcase material as textured sphere
                //data variables (showcase texture name instead of map id)
                // toggle between mapID and standard slider (vec3/float)

                if (ImGui::CollapsingHeader("Maps", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::BeginTable("##maps", 6, ImGuiTableFlags_SizingFixedFit);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                    InputAssetWidget<CONSTANTS::DND_PAYLOAD_TEXTURE>("albedo map", mat->albedoMapID);
                    InputAssetWidget<CONSTANTS::DND_PAYLOAD_TEXTURE>("normal map", mat->normalMapID);
                    InputAssetWidget<CONSTANTS::DND_PAYLOAD_TEXTURE>("roughness map", mat->roughnessMapID);
                    InputAssetWidget<CONSTANTS::DND_PAYLOAD_TEXTURE>("metallic map", mat->metallicMapID);
                    InputAssetWidget<CONSTANTS::DND_PAYLOAD_TEXTURE>("occlusion map", mat->occlusionMapID);
                    InputAssetWidget<CONSTANTS::DND_PAYLOAD_TEXTURE>("emissive map", mat->emissiveMapID);
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
            else {
                ImGui::Button("nothing here!");
            }
            });
    }

    void InspectorPanel::DeleteUpdate() {
        if ((m_App->SelectedEntity() != entt::null ||
            m_App->SelectedAsset().id != 0u) &&
            ImGui::IsKeyPressed(ImGuiKey_Delete, false))
        {
            showDeletePopup = true;
        }
        if (showDeletePopup) {
            ImGui::OpenPopup("Confirm Delete");
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

            if (ImGui::BeginPopupModal("Confirm Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                AppInterface::AssetInfo info{};

                if (m_App->SelectedEntity() != entt::null) {
                    Boom::Entity selectedEntity{ &m_App->GetEntityRegistry(), m_App->SelectedEntity() };
                    info.name = selectedEntity.Get<Boom::InfoComponent>().name;
                    info.id = selectedEntity.Get<Boom::InfoComponent>().uid;
                }
                else if (m_App->SelectedAsset().id != 0u) {
                    info = m_App->SelectedAsset();
                }

                ImGui::Text("Are you sure you want to delete:\n%s?", info.name.c_str());
                ImGui::Separator();
                if (ImGui::Button("Yes", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
                    if (m_App->SelectedEntity() != entt::null) {
                        m_App->GetEntityRegistry().destroy(m_App->SelectedEntity());
                        m_App->ResetAllSelected();
                    }
                    else if (m_App->SelectedAsset().id != 0u) {
                        m_App->DeleteAsset(info.id, info.type);
                        m_App->ResetAllSelected();
                    }
                    showDeletePopup = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("No", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    showDeletePopup = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }

    void InspectorPanel::AcceptIDDrop(uint64_t& data, char const* payloadType) {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType))
            {
                IM_ASSERT(payload->DataSize == sizeof(AssetID));
                data = *(AssetID const*)payload->Data;
                ImGui::Text("Dropped ID: %llu", data);
            }
            ImGui::EndDragDropTarget();
        }
    }

    template <std::string_view const& Payload>
    void InspectorPanel::InputAssetWidget(char const* label, uint64_t& data) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(label);
        ImGui::SameLine();

        ImGui::TableSetColumnIndex(1);
        ImVec2 const fieldSize{ ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() };
        ImGui::PushID(label);

        using AssetType = typename PayloadToType<Payload>::Type;
        if (ImGui::Button(m_App->GetAssetName<AssetType>(data).data(), fieldSize)) {
            //TODO: clicking button opens asset picker window
        }
        AcceptIDDrop(data, Payload.data());
        ImGui::PopID();
    }

} // namespace EditorUI
