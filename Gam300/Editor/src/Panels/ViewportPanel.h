#pragma once
#include <cstdint>                 // for uint32_t
#include "Vendors/imgui/imgui.h"   // for ImTextureID, ImVec2
#include "Vendors/imGuizmo/ImGuizmo.h"
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

        // Fullscreen controls
        void SetFullscreen(bool fullscreen) { m_IsFullscreen = fullscreen; }
        bool IsFullscreen() const { return m_IsFullscreen; }
        void ToggleFullscreen() { m_IsFullscreen = !m_IsFullscreen; }

    private:
        // helpers
        std::uint32_t QuerySceneFrame() const;   // prefers AppInterface::GetSceneFrame()
        double        QueryDeltaTime() const;    // prefers AppInterface::GetDeltaTime()

    private:
        Editor* m_Owner = nullptr;  // non-owning
        Boom::AppInterface* m_App = nullptr;  // optionally sourced from owner
        Boom::AppContext* m_Ctx = nullptr;  // optionally cached via owner

        bool         m_ShowViewport = true;
        bool         m_IsFullscreen = false;  // ADD THIS LINE

        ImTextureID  m_Frame = (ImTextureID)0;  // set in ctor
        std::uint32_t m_FrameId = 0;
        ImVec2       m_Viewport{ 0.0f, 0.0f };

        int m_GizmoOperation = ImGuizmo::TRANSLATE;
        int m_GizmoMode = ImGuizmo::WORLD;
        bool m_UseSnap = false;
        float m_SnapValues[3] = { 1.0f, 15.0f, 0.5f };
    };

} // namespace EditorUI