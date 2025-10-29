#pragma once
#include "Context/Context.h"
#include "Vendors/imgui/imgui.h"

// Adjust this include to where your Application + ApplicationState are declared.
// (If you already include Application in a PCH, you can remove this.)
#include "AppWindow.h" // defines Application, ApplicationState

// Dockable panel that controls app Run/Pause/Stop and shows time info.
class PlaybackControlsPanel : public IWidget
{
public:
    BOOM_INLINE explicit PlaybackControlsPanel(AppInterface* ctx, Boom::Application* app = nullptr);

    // Render per-frame
    void Render();                // wrapper -> calls OnShow()
    BOOM_INLINE void OnShow() override;

    // Visibility
    BOOM_INLINE void Show(bool v) { m_ShowPlaybackControls = v; }
    BOOM_INLINE bool IsVisible() const { return m_ShowPlaybackControls; }

    // App wiring (if you construct before Application exists)
    BOOM_INLINE void SetApplication(Application* app) { m_Application = app; }

private:
    bool         m_ShowPlaybackControls = true;
    Application* m_Application = nullptr;
};
