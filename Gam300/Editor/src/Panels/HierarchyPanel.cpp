#include "Panels/HierarchyPanel.h"

// pull full types here (keeps header light)
#include "Editor/Editor.h"
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "BoomEngine.h"            // for Boom::InfoComponent (adjust include if needed)

#include <entt/entt.hpp>

namespace EditorUI {

    HierarchyPanel::HierarchyPanel(Editor* owner)
        : m_Owner(owner)
    {
        DEBUG_DLL_BOUNDARY("HierarchyPanel::Constructor");

        if (!m_Owner) {
            BOOM_ERROR("HierarchyPanel::Constructor - Null owner!");
            return;
        }
        // Editor must provide: Boom::AppContext* GetContext() const;
        m_Ctx = m_Owner->GetContext();
        DEBUG_POINTER(m_Ctx, "AppContext");

        if (!m_Ctx) {
            BOOM_ERROR("HierarchyPanel::Constructor - Null AppContext!");
            return;
        }

        // If your Editor exposes flags/selection, wire them here (optional):
        // m_ShowHierarchy  = &m_Owner->m_ShowHierarchy;
        // m_SelectedEntity = &m_Owner->m_SelectedEntity;
    }

    void HierarchyPanel::Render()
    {
        if (!m_Ctx) return;

        // If no external flag wired, treat as visible
        bool open_local = true;
        bool* p_open = m_ShowHierarchy ? m_ShowHierarchy : &open_local;

        if (ImGui::Begin("Hierarchy", p_open))
        {
            ImGui::TextUnformatted("Scene Hierarchy");
            ImGui::Separator();

            // Prefer a getter; fallback to public .scene if that’s your API
            // auto& registry = m_Ctx->scene;
            auto& registry = m_Ctx->GetRegistry(); // adjust to your actual API


            auto view = registry.view<Boom::InfoComponent>();
            for (entt::entity e : view)
            {
                const auto& info = view.get<Boom::InfoComponent>(e);
                const bool isSelected = (m_SelectedEntity && *m_SelectedEntity == e);

                ImGui::PushID(static_cast<int>(entt::to_integral(e)));
                if (ImGui::Selectable(info.name.c_str(), isSelected))
                {
                    if (m_SelectedEntity) *m_SelectedEntity = e;
                    BOOM_INFO("[Hierarchy] Selected entity: {}", info.name);
                }
                ImGui::PopID();
            }
        }
        ImGui::End();
    }

} // namespace EditorUI
