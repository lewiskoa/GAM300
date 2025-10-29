#pragma once

#include <entt/entity/entity.hpp> // entt::entity

namespace Boom { struct Context; }

class HierarchyPanel
{
public:
    HierarchyPanel() = default;
    explicit HierarchyPanel(Boom::Context* ctx, bool* showFlag = nullptr, entt::entity* selected = nullptr)
        : m_Context(ctx), m_ShowHierarchy(showFlag), m_SelectedEntity(selected) {}

    void Render();

    void SetContext(Boom::Context* ctx) { m_Context = ctx; }
    void SetShowFlag(bool* flag) { m_ShowHierarchy = flag; }
    void SetSelectedEntity(entt::entity* sel) { m_SelectedEntity = sel; }

private:
    Boom::Context* m_Context{ nullptr };     // NOTE: fully-qualified type
    bool* m_ShowHierarchy{ nullptr };
    entt::entity* m_SelectedEntity{ nullptr };
};