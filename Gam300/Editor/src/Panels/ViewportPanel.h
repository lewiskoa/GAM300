#pragma once

#include <cstdint>

// ImGui types
#include "Vendors/imgui/imgui.h"

namespace Boom {
    class AppContext; // full type included in .cpp
}

namespace EditorUI {

    class Editor; // forward-declared so header stays light

    class ViewportPanel {
    public:
        explicit ViewportPanel(Editor* owner);

        // Editor.cpp calls this:
        void Render();

        // You can call these from elsewhere if needed
        void OnShow();
        void OnSelect(uint32_t entity_id);   // keep signature simple & decoupled
        void DebugViewportState() const;

        void Show(bool v) { m_ShowViewport = v; }
        bool IsVisible() const { return m_ShowViewport; }

    private:
        Editor* m_Owner = nullptr;   // non-owning
        Boom::AppContext* m_Ctx = nullptr;   // cached from Editor
        bool               m_ShowViewport = true;

        ImTextureID        m_Frame = {};        // zero-init
        uint32_t           m_FrameId = 0;
        ImVec2             m_Viewport = ImVec2(0.0f, 0.0f);
    };

} // namespace EditorUI
