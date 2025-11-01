#pragma once

#include <cstdint>                 // for uint32_t
#include "Vendors/imgui/imgui.h"   // for ImTextureID, ImVec2
#include "ImGuizmo.h"
#include <glm/gtc/type_ptr.hpp>

namespace Boom { struct AppContext; }
namespace Boom { struct AppInterface; }

namespace EditorUI {

    class Editor;

    class ViewportPanel {
    public:
        explicit ViewportPanel(Editor* owner);

        void Render();
        void OnShow();
        void OnSelect(std::uint32_t entity_id);
        void DebugViewportState() const;

        void Show(bool v) { m_ShowViewport = v; }
        bool IsVisible() const { return m_ShowViewport; }
        ImVec2 GetSize() const { return m_Viewport; }

    private:
        // helpers
        std::uint32_t QuerySceneFrame() const;   // prefers AppInterface::GetSceneFrame()
        double        QueryDeltaTime() const;    // prefers AppInterface::GetDeltaTime()

    private:
        Editor* m_Owner = nullptr;  // non-owning
        Boom::AppInterface* m_App = nullptr;  // optionally sourced from owner
        Boom::AppContext* m_Ctx = nullptr;  // optionally cached via owner

        bool         m_ShowViewport = true;
        ImTextureID  m_Frame = (ImTextureID)0;  // set in ctor
        std::uint32_t m_FrameId = 0;
        ImVec2       m_Viewport{ 0.0f, 0.0f };


    };

} // namespace EditorUI

