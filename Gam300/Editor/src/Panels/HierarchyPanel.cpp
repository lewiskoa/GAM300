#include "Panels/HierarchyPanel.h"

// ImGui
#include "Vendors/imgui/imgui.h"

// Engine includes (adjust to match your structure)
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "BoomEngine.h"

void HierarchyPanel::Render()
{
    if (!m_ShowHierarchy || !*m_ShowHierarchy)
        return;

    if (!m_Context)
        return;

    if (ImGui::Begin("Hierarchy", m_ShowHierarchy))
    {
        ImGui::TextUnformatted("Scene Hierarchy");
        ImGui::Separator();

        // Fetch entities with InfoComponent
        auto view = m_Context->scene.view<Boom::InfoComponent>();

        for (auto entityID : view)
        {
            auto& info = view.get<Boom::InfoComponent>(entityID);

            bool isSelected = (m_SelectedEntity && *m_SelectedEntity == entityID);

            // Use entity ID to ensure ImGui item IDs are unique
            ImGui::PushID(static_cast<int>(entityID));

            if (ImGui::Selectable(info.name.c_str(), isSelected))
            {
                if (m_SelectedEntity)
                    *m_SelectedEntity = entityID;

                BOOM_INFO("[Hierarchy] Selected entity: {}", info.name);
            }

            ImGui::PopID();
        }
    }

    ImGui::End();
}
