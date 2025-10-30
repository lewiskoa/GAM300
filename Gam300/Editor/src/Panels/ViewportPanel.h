#pragma once

#include "Vendors/imgui/imgui.h"

namespace Boom { struct AppContext; struct AppInterface; }

namespace EditorUI {

    class Editor;

    class ViewportPanel {
    public:
        explicit ViewportPanel(Editor* owner);

        void Render();
        void OnShow();
        void OnSelect(uint32_t entity_id);
        void DebugViewportState() const;

        void Show(bool v) { m_ShowViewport = v; }
        bool IsVisible() const { return m_ShowViewport; }

    private:
        // helpers
        uint32_t QuerySceneFrame() const;   // prefers AppInterface::GetSceneFrame()
        double   QueryDeltaTime() const;    // prefers AppInterface::GetDeltaTime()

    private:
        Editor* m_Owner = nullptr;   // non-owning
        Boom::AppInterface* m_App = nullptr;   // sourced from owner if possible
        Boom::AppContext* m_Ctx = nullptr;   // optional cache (owner->GetContext())

        bool        m_ShowViewport = true;
        ImTextureID m_Frame{};                   // set in ctor
        uint32_t    m_FrameId = 0;
        ImVec2      m_Viewport{ 0.0f, 0.0f };
    };

} // namespace EditorUI
