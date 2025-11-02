#pragma once
#pragma once
#include <functional>
#include <entt/entt.hpp>
#include "Vendors/imgui/imgui.h"

namespace Boom { struct AppContext; }

namespace EditorUI {

    class Editor;  // forward declare the owner (your Editor class)

    class InspectorPanel
    {
    public:
        // Preferred: construct with the Editor owner; get context via owner->GetContext()
        explicit InspectorPanel(Editor* owner, bool* showFlag = nullptr)
            : m_Owner(owner), m_ShowInspector(showFlag) {}

        // Optional helper if you really want to set selection externally
        void SetSelectedEntity(entt::entity e) { m_SelectedEntity = e; m_HasSelection = (e != entt::null); }
        void ClearSelection() { m_SelectedEntity = entt::null; m_HasSelection = false; }

        // Render entry point
        void Render();

        // Accessors
        Boom::AppContext* GetContext() const;   // implemented in .cpp using m_Owner->GetContext()
        Editor* GetOwner() const { return m_Owner; }
        void SetShowFlag(bool* flag) { m_ShowInspector = flag; }

    private:
        Editor* m_Owner = nullptr;        // we avoid storing AppContext directly here
        bool* m_ShowInspector = nullptr;
        entt::entity   m_SelectedEntity{ entt::null };
        bool           m_HasSelection{ false };
        char           m_NameBuffer[128]{};

        template<typename TComponent, typename GetPropsFn>
        void DrawComponentSection(const char* title,
            TComponent* comp,
            GetPropsFn getProps,
            bool removable,
            const std::function<void()>& onRemove);
    };

}
