#include "Panels/HierarchyPanel.h"

#include "Vendors/imgui/imgui.h"
#include <entt/entt.hpp>                 // for view(), to_integral, etc.

// Must provide full definitions here:
#include "Context/Context.h"

#include "Context/DebugHelpers.h"
#include "BoomEngine.h"

void HierarchyPanel::Render()
{
    // Treat missing flag as "visible"
    if (m_ShowHierarchy && !*m_ShowHierarchy) return;
    if (!m_Context) return;

    bool open = m_ShowHierarchy ? *m_ShowHierarchy : true;
    if (ImGui::Begin("Hierarchy", m_ShowHierarchy ? m_ShowHierarchy : &open))
    {
        ImGui::TextUnformatted("Scene Hierarchy");
        ImGui::Separator();

        // Either access the public field...
        // auto& registry = m_Context->scene;

        // ...or if your Context exposes a getter, use that (prefer this if 'scene' isn't public):
        auto& registry = m_Context->GetRegistry(); // <-- change to your actual API

        auto view = registry.view<Boom::InfoComponent>();
        for (entt::entity e : view)
        {
            auto& info = view.get<Boom::InfoComponent>(e);
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
