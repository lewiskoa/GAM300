#pragma once
#include "Vendors/imgui/imgui.h"

namespace Boom { class AppContext; class Application; enum class ApplicationState : int; }

namespace EditorUI {

    class Editor;

    class PlaybackControlsPanel {
    public:
        explicit PlaybackControlsPanel(Editor* owner, Boom::Application* app = nullptr);

        void Render();              // Editor.cpp calls this
        void OnShow();              // internal

        void Show(bool v) { m_Show = v; }
        bool IsVisible() const { return m_Show; }
        void SetApplication(Boom::Application* app) { m_App = app; }

    private:
        Editor* m_Owner = nullptr;
        Boom::AppContext* m_Ctx = nullptr;
        Boom::Application* m_App = nullptr;
        bool              m_Show = true;
    };

} // namespace EditorUI
