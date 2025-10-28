#pragma once
#pragma once

#include <entt/entity/entity.hpp>
#include <memory>

// Forward declarations to keep compile times low
struct Context;

class HierarchyPanel
{
public:
    HierarchyPanel() = default;
    explicit HierarchyPanel(Context* ctx, bool* showFlag = nullptr, entt::entity* selected = nullptr)
        : m_Context(ctx), m_ShowHierarchy(showFlag), m_SelectedEntity(selected)
    {}

    // Draws the Hierarchy window
    void Render();

    // Optional manual setters
    void SetContext(Context* ctx) { m_Context = ctx; }
    void SetShowFlag(bool* flag) { m_ShowHierarchy = flag; }
    void SetSelectedEntity(entt::entity* sel) { m_SelectedEntity = sel; }

private:
    Context* m_Context{ nullptr };          // Pointer to engine/editor context (contains scene)
    bool* m_ShowHierarchy{ nullptr };    // Visibility flag
    entt::entity* m_SelectedEntity{ nullptr };   // Currently selected entity ID
};
