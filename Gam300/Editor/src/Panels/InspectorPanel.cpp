// Panels/InspectorPanel.cpp
#include "Panels/InspectorPanel.h"
#include "Editor.h"          // for Editor::GetContext()
#include "Context/Context.h"        // for Boom::AppContext + scene access
#include "Vendors/imgui/imgui.h"
#include "Auxiliaries/Assets.h"
#include "Context/DebugHelpers.h"
#include "Panels/PropertiesImgui.h"
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

        const ImVec2 headerMin = ImGui::GetItemRectMin();
        const ImVec2 headerMax = ImGui::GetItemRectMax();
        const float  lineH = ImGui::GetFrameHeight();

        // right-align the "..." inside the header, unique popup per component
        ImGui::PushID(comp);
        if (removable) {
            const float y = headerMin.y + (headerMax.y - headerMin.y - lineH) * 0.5f;
            ImGui::SetCursorScreenPos(ImVec2(headerMax.x - lineH, y));
            if (ImGui::Button("...", ImVec2(lineH, lineH)))
                ImGui::OpenPopup("ComponentSettings");

            if (ImGui::BeginPopup("ComponentSettings")) {
                if (ImGui::MenuItem("Remove Component")) {
                    if (onRemove) onRemove();
                    ImGui::EndPopup();
                    if (open) ImGui::TreePop();
                    ImGui::PopID();
                    return;
                }
                ImGui::EndPopup();
            }
        }
        ImGui::PopID();

        ImGui::SetCursorScreenPos(ImVec2(headerMin.x, headerMax.y + ImGui::GetStyle().ItemSpacing.y));

        if (open) {
            using SchemaT = const xproperty::type::object*;
            SchemaT schema = nullptr;

            // Try all reasonable call forms; your macro declares: SchemaT GetXxx(void*)
            if constexpr (std::is_invocable_r_v<SchemaT, GetPropsFn, void*>) {
                schema = getProps(static_cast<void*>(comp));
            }
            else if constexpr (std::is_invocable_r_v<SchemaT, GetPropsFn, TComponent*>) {
                schema = getProps(comp);
            }
            else if constexpr (std::is_invocable_r_v<SchemaT, GetPropsFn, const TComponent*>) {
                schema = getProps(static_cast<const TComponent*>(comp));
            }
            else if constexpr (std::is_invocable_r_v<SchemaT, GetPropsFn, TComponent&>) {
                schema = getProps(*comp);
            }
            else if constexpr (std::is_invocable_r_v<SchemaT, GetPropsFn, const TComponent&>) {
                schema = getProps(static_cast<const TComponent&>(*comp));
            }
            else {
                // (Fallback: if someone passes a void-returning drawer)
                if constexpr (std::is_invocable_v<GetPropsFn, void*>)               getProps(static_cast<void*>(comp));
                else if constexpr (std::is_invocable_v<GetPropsFn, TComponent*>)    getProps(comp);
                else if constexpr (std::is_invocable_v<GetPropsFn, const TComponent*>) getProps(static_cast<const TComponent*>(comp));
                else if constexpr (std::is_invocable_v<GetPropsFn, TComponent&>)    getProps(*comp);
                else if constexpr (std::is_invocable_v<GetPropsFn, const TComponent&>) getProps(static_cast<const TComponent&>(*comp));
            }

            // If we got a schema, render it with your UI bridge
            if (schema) {
                DrawPropertiesUI(schema, static_cast<void*>(comp));
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
        Boom::Entity selected{ &ctx->scene, m_App->SelectedEntity() };

        // ===== ENTITY NAME =====
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
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        // ===== COMPONENTS =====
        if (selected.Has<Boom::TransformComponent>()) {
            auto& tc = selected.Get<Boom::TransformComponent>();
            if (ImGui::CollapsingHeader("Model Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
                //modified as dragging speed should vary between variables
                ImGui::DragFloat3("Translate", &tc.transform.translate[0], 0.01f); 
                ImGui::DragFloat3("Rotation", &tc.transform.rotate[0], .3142f);
                ImGui::DragFloat3("Scale", &tc.transform.scale[0], 0.01f);
                tc.transform.scale = glm::max(glm::vec3(0.01f), tc.transform.scale); //limit scale to positive
            }
        }

        if (selected.Has<Boom::CameraComponent>()) {
            auto& cc = selected.Get<Boom::CameraComponent>();
            DrawComponentSection("Camera", &cc, GetCameraComponentProperties, true,
                [&]() { ctx->scene.remove<Boom::CameraComponent>(m_App->SelectedEntity()); });
        }

        // Model Component
        if (selected.Has<Boom::ModelComponent>()) {
            auto& mc = selected.Get<Boom::ModelComponent>();

            // Use CollapsingHeader to match the style
            if (ImGui::CollapsingHeader("Model Renderer", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap)) {
                ComponentSettings<Boom::ModelComponent>(ctx);

                // --- UI for assigning model and material ---
                ImGui::BeginTable("##maps", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV);
                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Asset", ImGuiTableColumnFlags_WidthStretch);
                InputAssetWidget<CONSTANTS::DND_PAYLOAD_MODEL>("Model", mc.modelID);
                InputAssetWidget<CONSTANTS::DND_PAYLOAD_MATERIAL>("Material", mc.materialID);
                ImGui::EndTable();

                ImGui::Spacing();
                ImGui::SeparatorText("Physics"); // A nice separator

                // --- UI for cooking the mesh collider ---
                if (mc.modelID != EMPTY_ASSET) {
                    ModelAsset& modelAsset = m_App->GetAssetRegistry().Get<ModelAsset>(mc.modelID);

                    if (modelAsset.data) {
                        if (ImGui::Button("Compile Mesh Collider from this Model", ImVec2(-1, 0))) {
                            std::string saveDir = "Resources/Physics/";
                            if (!std::filesystem::exists(saveDir)) {
                                std::filesystem::create_directories(saveDir);
                            }
                            // Use model name for the .pxm file
                            std::string savePath = saveDir + modelAsset.name + ".pxm";

                            bool success = m_App->GetPhysicsContext().CompileAndSavePhysicsMesh(modelAsset, savePath);

                            if (success) {
                                AssetID newID = RandomU64(); // Assumes RandomU64() is available
                                m_App->GetAssetRegistry().AddPhysicsMesh(newID, savePath)->name = modelAsset.name;
                                BOOM_INFO("Successfully cooked and created PhysicsMeshAsset '{}'", modelAsset.name);
                                m_App->SaveAssets();
                            }
                            else {
                                BOOM_ERROR("Failed to cook physics mesh for '{}'. Check model data.", modelAsset.name);
                            }
                        }
                    }
                    else {
                        ImGui::TextDisabled("Model data not yet loaded.");
                    }
                }
                else {
                    ImGui::TextDisabled("Assign a model to enable mesh cooking.");
                }
            }
        }

        if (selected.Has<Boom::RigidBodyComponent>()) {
            ImGui::PushID("Rigid Body");

            // 1. Draw Header
            bool isOpen = ImGui::CollapsingHeader("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);

            // 2. Draw "..." Button (to match photo)
            const ImVec2 headerMin = ImGui::GetItemRectMin();
            const ImVec2 headerMax = ImGui::GetItemRectMax();
            const float  lineH = ImGui::GetFrameHeight();
            const float y = headerMin.y + (headerMax.y - headerMin.y - lineH) * 0.5f;
            ImGui::SetCursorScreenPos(ImVec2(headerMax.x - lineH, y));
            if (ImGui::Button("...", ImVec2(lineH, lineH)))
                ImGui::OpenPopup("RigidBodySettings");

            bool removed = false;
            if (ImGui::BeginPopup("RigidBodySettings")) {
                if (ImGui::MenuItem("Remove Component")) {
                    removed = true; // Set flag to remove later
                }
                ImGui::EndPopup();
            }

            // 3. Reset cursor
            ImGui::SetCursorScreenPos(ImVec2(headerMin.x, headerMax.y + ImGui::GetStyle().ItemSpacing.y));

            // 4. Draw Contents
            if (isOpen) {
                ImGui::Indent(12.0f);
                ImGui::Spacing();

                auto& rc = selected.Get<Boom::RigidBodyComponent>();

                RigidBody3D::Type currentType = rc.RigidBody.type;
                const char* currentTypeName;
                switch (currentType)
                {
                case RigidBody3D::Type::STATIC:  currentTypeName = "Static";  break;
                case RigidBody3D::Type::DYNAMIC: currentTypeName = "Dynamic"; break;
                default:                         currentTypeName = "Unknown"; break;
                }

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Body Type");
                ImGui::SameLine(150);
                ImGui::SetNextItemWidth(-1);

                if (ImGui::BeginCombo("##BodyType", currentTypeName))
                {
                    bool isStaticSelected = (currentType == RigidBody3D::Type::STATIC);
                    if (ImGui::Selectable("Static", isStaticSelected))
                    {
                        m_App->GetPhysicsContext().SetRigidBodyType(selected, RigidBody3D::Type::STATIC);
                    }
                    if (isStaticSelected) ImGui::SetItemDefaultFocus();

                    bool isDynamicSelected = (currentType == RigidBody3D::Type::DYNAMIC);
                    if (ImGui::Selectable("Dynamic", isDynamicSelected))
                    {
                        m_App->GetPhysicsContext().SetRigidBodyType(selected, RigidBody3D::Type::DYNAMIC);
                    }
                    if (isDynamicSelected) ImGui::SetItemDefaultFocus();

                    ImGui::EndCombo();
                }

                auto* rigidBody = &rc.RigidBody;

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Density");
                ImGui::SameLine(150);
                ImGui::SetNextItemWidth(-1);
                ImGui::DragFloat("##Density", &rigidBody->density, 0.01f, 0.0f, 1000.0f);

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Mass");
                ImGui::SameLine(150);
                ImGui::SetNextItemWidth(-1);
                ImGui::DragFloat("##Mass", &rigidBody->mass, 0.1f, 0.0f, 1000.0f);

                ImGui::Spacing();
                ImGui::Unindent(12.0f);
            }

            ImGui::PopID();

            if (removed) {
                ctx->scene.remove<Boom::RigidBodyComponent>(m_App->SelectedEntity());
                return; // Exit EntityUpdate early
            }
            ImGui::Spacing();
        }

        if (selected.Has<Boom::ColliderComponent>()) {
            ImGui::PushID("Collider");

            // 1. Draw Header
            bool isOpen = ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);

            // 2. Draw "..." Button
            const ImVec2 headerMin = ImGui::GetItemRectMin();
            const ImVec2 headerMax = ImGui::GetItemRectMax();
            const float  lineH = ImGui::GetFrameHeight();
            const float y = headerMin.y + (headerMax.y - headerMin.y - lineH) * 0.5f;
            ImGui::SetCursorScreenPos(ImVec2(headerMax.x - lineH, y));
            if (ImGui::Button("...", ImVec2(lineH, lineH)))
                ImGui::OpenPopup("ColliderSettings");

            bool removed = false;
            if (ImGui::BeginPopup("ColliderSettings")) {
                if (ImGui::MenuItem("Remove Component")) {
                    removed = true;
                }
                ImGui::EndPopup();
            }

            // 3. Reset cursor
            ImGui::SetCursorScreenPos(ImVec2(headerMin.x, headerMax.y + ImGui::GetStyle().ItemSpacing.y));

            // 4. Draw Contents
            if (isOpen) {
                ImGui::Indent(12.0f);
                ImGui::Spacing();

                auto& col = selected.Get<Boom::ColliderComponent>();

                float oldDynamicFriction = col.Collider.dynamicFriction;
                float oldStaticFriction = col.Collider.staticFriction;
                float oldRestitution = col.Collider.restitution;

                Collider3D::Type currentType = col.Collider.type;
                const char* currentTypeName = "Unknown";
                switch (currentType)
                {
                case Collider3D::Type::BOX:     currentTypeName = "Box";     break;
                case Collider3D::Type::SPHERE:  currentTypeName = "Sphere";  break;
                case Collider3D::Type::CAPSULE: currentTypeName = "Capsule"; break;
                case Collider3D::Type::MESH:    currentTypeName = "Mesh";    break;
                }

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Shape Type");
                ImGui::SameLine(150);
                ImGui::SetNextItemWidth(-1);

                if (ImGui::BeginCombo("##ColliderType", currentTypeName))
                {
                    const char* types[] = { "Box", "Sphere", "Capsule", "Mesh" };
                    for (int i = 0; i < IM_ARRAYSIZE(types); ++i) {
                        bool isSelected = (currentType == static_cast<Collider3D::Type>(i));
                        if (ImGui::Selectable(types[i], isSelected)) {
                            m_App->GetPhysicsContext().SetColliderType(selected, static_cast<Collider3D::Type>(i), m_App->GetAssetRegistry());
                        }
                        if (isSelected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (currentType == Collider3D::Type::MESH)
                {
                    ImGui::Spacing();
                    ImGui::Separator();

                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Physics Mesh");
                    ImGui::SameLine(150);
                    ImGui::SetNextItemWidth(-1);

                    auto& assetRegistry = m_App->GetAssetRegistry();
                    auto& currentAsset = assetRegistry.Get<PhysicsMeshAsset>(col.Collider.physicsMeshID);
                    const char* currentName = (currentAsset.uid != EMPTY_ASSET) ? currentAsset.name.c_str() : "Select a mesh...";

                    if (ImGui::BeginCombo("##PhysicsMesh", currentName))
                    {
                        auto& map = assetRegistry.GetMap<PhysicsMeshAsset>();

                        bool isNoneSelected = (col.Collider.physicsMeshID == EMPTY_ASSET);
                        if (ImGui::Selectable("None", isNoneSelected))
                        {
                            col.Collider.physicsMeshID = EMPTY_ASSET;
                            m_App->GetPhysicsContext().UpdateColliderShape(selected, m_App->GetAssetRegistry());
                        }
                        if (isNoneSelected) ImGui::SetItemDefaultFocus();

                        for (auto& [uid, asset] : map)
                        {
                            if (uid == EMPTY_ASSET) continue;
                            bool isSelected = (col.Collider.physicsMeshID == uid);
                            if (ImGui::Selectable(asset->name.c_str(), isSelected))
                            {
                                col.Collider.physicsMeshID = uid;
                                m_App->GetPhysicsContext().UpdateColliderShape(selected, m_App->GetAssetRegistry());
                            }
                            if (isSelected) ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::Separator();
                    ImGui::Spacing();
                }

                auto* collider = &col.Collider;

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Dynamic Friction");
                ImGui::SameLine(150);
                ImGui::SetNextItemWidth(-1);
                ImGui::DragFloat("##DynamicFriction", &collider->dynamicFriction, 0.01f, 0.0f, 100.0f);

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Static Friction");
                ImGui::SameLine(150);
                ImGui::SetNextItemWidth(-1);
                ImGui::DragFloat("##StaticFriction", &collider->staticFriction, 0.01f, 0.0f, 100.0f);

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Restitution");
                ImGui::SameLine(150);
                ImGui::SetNextItemWidth(-1);
                ImGui::DragFloat("##Restitution", &collider->restitution, 0.01f, 0.0f, 100.0f);

                if (col.Collider.dynamicFriction != oldDynamicFriction ||
                    col.Collider.staticFriction != oldStaticFriction ||
                    col.Collider.restitution != oldRestitution)
                {
                    m_App->GetPhysicsContext().UpdatePhysicsMaterial(selected);
                }

                ImGui::Spacing();
                ImGui::Unindent(12.0f);
            }
            ImGui::PopID();

            if (removed) {
                ctx->scene.remove<Boom::ColliderComponent>(m_App->SelectedEntity());
                return; // Exit EntityUpdate early
            }
            ImGui::Spacing();
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
        ComponentSelector(selected);
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
            else if (asset->type == AssetType::MODEL) {
                ModelAsset* m{ dynamic_cast<ModelAsset*>(asset) };
                //TODO: showcase model without texture

                if (ImGui::CollapsingHeader("Model Offset", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::DragFloat3("Translate", &m->data->modelTransform.translate[0], 0.01f);
                    ImGui::DragFloat3("Rotation", &m->data->modelTransform.rotate[0], 1.f, 0.f, 360.f);
                    ImGui::DragFloat3("Scale", &m->data->modelTransform.scale[0], 0.01f, 0.01f);
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

    void InspectorPanel::ComponentSelector(Boom::Entity& selected) {
        if (ImGui::BeginPopup("AddComponentPopup")) {
            ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(500, 600));

            ImGui::Text("Select component to add:");
            ImGui::Separator();
            if (ImGui::BeginChild("ComponentScrollArea", ImVec2(0, 250), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
                if (ImGui::BeginTable("Component Table", 1, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
                    UpdateComponent<Boom::InfoComponent>(Boom::ComponentID::INFO, selected);
                    UpdateComponent<Boom::TransformComponent>(Boom::ComponentID::TRANSFORM, selected);
                    UpdateComponent<Boom::CameraComponent>(Boom::ComponentID::CAMERA, selected);
                    UpdateComponent<Boom::RigidBodyComponent>(Boom::ComponentID::RIGIDBODY, selected);
                    UpdateComponent<Boom::ColliderComponent>(Boom::ComponentID::COLLIDER, selected);
                    UpdateComponent<Boom::ModelComponent>(Boom::ComponentID::MODEL, selected);
                    UpdateComponent<Boom::AnimatorComponent>(Boom::ComponentID::ANIMATOR, selected);
                    UpdateComponent<Boom::DirectLightComponent>(Boom::ComponentID::DIRECT_LIGHT, selected);
                    UpdateComponent<Boom::PointLightComponent>(Boom::ComponentID::POINT_LIGHT, selected);
                    UpdateComponent<Boom::SpotLightComponent>(Boom::ComponentID::SPOT_LIGHT, selected);
                    UpdateComponent<Boom::SoundComponent>(Boom::ComponentID::SOUND, selected);
                    UpdateComponent<Boom::ScriptComponent>(Boom::ComponentID::SCRIPT, selected);
                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();
            ImGui::EndPopup();
        }
    }

    template <class Type> 
    void InspectorPanel::UpdateComponent(Boom::ComponentID id, Boom::Entity& selected) {
        if (!selected.Has<Type>()) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(static_cast<int>(id));
            if (ImGui::Selectable(COMPONENT_NAMES[static_cast<size_t>(id)].data())) { 
                selected.Attach<Type>();
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopID();
        }
    }

    template <class CType> 
    void InspectorPanel::ComponentSettings(Boom::AppContext* ctx) {
        const ImVec2 headerMin = ImGui::GetItemRectMin();
        const ImVec2 headerMax = ImGui::GetItemRectMax();
        const float  lineH = ImGui::GetFrameHeight();
        ImGui::SetCursorScreenPos(ImVec2(headerMax.x - lineH, headerMin.y + (headerMax.y - headerMin.y - lineH) * 0.5f));
        if (ImGui::Button("...", ImVec2(lineH, lineH)))
            ImGui::OpenPopup("ComponentSettings");
        if (ImGui::BeginPopup("ComponentSettings")) {
            if (ImGui::MenuItem("Remove Component")) {
                ctx->scene.remove<CType>(m_App->SelectedEntity());
            }
            ImGui::EndPopup();
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
