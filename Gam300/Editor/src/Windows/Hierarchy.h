#pragma once

#include "Context/Context.h"

struct HierarchyWindow : IWidget {
	BOOM_INLINE HierarchyWindow(AppInterface* c)
		: IWidget{ c }
	{
	}

	BOOM_INLINE void OnShow() override {
        if (!m_ShowHierarchy) return;

        if (ImGui::Begin("Hierarchy", &m_ShowHierarchy)) {
            ImGui::Text("Scene Hierarchy");
            ImGui::Separator();

            // Use the correct scene registry from the AppContext
            auto view = context->GetEntityRegistry().view<Boom::InfoComponent>();

            for (auto entityID : view) {
                auto& info = view.get<Boom::InfoComponent>(entityID);

                // Compare the raw entity IDs for selection
                bool isSelected = (context->SelectedEntity() == entityID);

                // Push the entity's unique ID onto ImGui's ID stack
                ImGui::PushID(static_cast<int>(entityID));

                // Now, you can safely use the (potentially non-unique) name for the label
                if (ImGui::Selectable(info.name.c_str(), isSelected)) {
                    // Assign the raw entity ID on click
                    context->SelectedEntity() = entityID;
                }

                // Pop the ID off the stack to keep it clean for the next item
                ImGui::PopID();
            }
        }
        ImGui::End();
	}

private:

public:
    bool m_ShowHierarchy{true};
};