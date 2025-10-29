#pragma once
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "Vendors/imgui/imgui.h"

// Performance overlay that displays FPS, frame time, and profiler information.
class PerformancePanel : public IWidget
{
public:
    BOOM_INLINE explicit PerformancePanel(AppInterface* ctx);

    // Called once per frame from your editor render loop
    void Render();               // wrapper for OnShow()
    BOOM_INLINE void OnShow() override;

    // Optional visibility control
    BOOM_INLINE void Show(bool v) { m_ShowPerformance = v; }
    BOOM_INLINE bool IsVisible() const { return m_ShowPerformance; }

private:
    static constexpr int kPerfHistory = 120;  // Number of frames stored in FPS graph
    bool   m_ShowPerformance = true;
    float  m_FpsHistory[kPerfHistory];
    int    m_FpsWriteIdx = 0;
};
