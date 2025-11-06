#pragma once
#pragma once
#include <functional>
#include <entt/entt.hpp>
#include "Vendors/imgui/imgui.h"
#include "GlobalConstants.h"
#include "ECS/ECS.hpp"

namespace Boom { struct AppContext; struct AppInterface; }

namespace EditorUI {

    class Editor;  // forward declare the owner (your Editor class)

    class InspectorPanel
    {
    public:
        // Preferred: construct with the Editor owner; get context via owner->GetContext()
        explicit InspectorPanel(Editor* owner, bool* showFlag = nullptr);

        // Render entry point
        void Render();

        // Accessors
        Boom::AppContext* GetContext() const;   // implemented in .cpp using m_Owner->GetContext()
        Editor* GetOwner() const { return m_Owner; }
        void SetShowFlag(bool* flag) { m_ShowInspector = flag; }
    
    private:
        // helpers
        void EntityUpdate();
        void AssetUpdate();
        void DeleteUpdate();
        void ComponentSelector(Boom::Entity& selected);
        template <class Type> void UpdateComponent(Boom::ComponentID id, Boom::Entity& selected);

        void AcceptIDDrop(uint64_t& data, char const* payloadType);
        template <std::string_view const& Payload>
        void InputAssetWidget(char const* label, uint64_t& data);

    private:
        Editor* m_Owner = nullptr;     
        Boom::AppInterface* m_App = nullptr;
        bool* m_ShowInspector = nullptr;
        bool showDeletePopup{};
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
