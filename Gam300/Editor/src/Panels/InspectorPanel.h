#pragma once

#include <functional>
#include <entt/entity/entity.hpp>

// Forward declarations to keep compile times down
struct Context;

class InspectorPanel
{
public:
    InspectorPanel() = default;
    InspectorPanel(Context* ctx, entt::entity* selected, bool* showFlag = nullptr)
        : m_Context(ctx), m_SelectedEntity(selected), m_ShowInspector(showFlag) {}

    void Render();

    // Optional wiring if you default-construct
    void SetContext(Context* ctx) { m_Context = ctx; }
    void SetSelectedEntity(entt::entity* entity) { m_SelectedEntity = entity; }
    void SetShowFlag(bool* flag) { m_ShowInspector = flag; }

private:
    template<typename TComponent, typename GetPropsFn>
    void DrawComponentSection(const char* title,
        TComponent* comp,
        GetPropsFn getProps,
        bool removable,
        const std::function<void()>& onRemove);

private:
    Context* m_Context{ nullptr };         // provides .scene (entt::registry) etc.
    entt::entity* m_SelectedEntity{ nullptr };  // selected entity from hierarchy
    bool* m_ShowInspector{ nullptr };   // visibility toggle (optional)

    // scratch buffer for the “Entity” name field
    char           m_NameBuffer[256]{};
};
