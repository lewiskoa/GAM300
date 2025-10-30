#pragma once

#include <entt/entity/entity.hpp> // entt::entity
#include "Vendors/imgui/imgui.h"

namespace Boom { class AppContext; }

namespace EditorUI {

    class Editor; // forward declare, full type only needed in .cpp

    class HierarchyPanel {
    public:
        explicit HierarchyPanel(Editor* owner);

        void Render();

        // Optional wiring from Editor
        void SetShowFlag(bool* flag) { m_ShowHierarchy = flag; }
        void SetSelectedEntity(entt::entity* sel) { m_SelectedEntity = sel; }

    private:
        Editor* m_Owner = nullptr;  // non-owning
        Boom::AppContext* m_Ctx = nullptr;  // cached from Editor
        bool* m_ShowHierarchy = nullptr;  // external toggle
        entt::entity* m_SelectedEntity = nullptr;  // selection binding
    };

} // namespace EditorUI
