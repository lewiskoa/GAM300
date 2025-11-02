#pragma once
#include "Vendors/imgui/imgui.h"

namespace Boom { struct AppContext; }

namespace EditorUI {

    class Editor;

    class PerformancePanel {
    public:
        explicit PerformancePanel(Editor* owner);
        void Render();          // Editor.cpp calls this
        void OnShow();          // internal

        void Show(bool v) { m_Show = v; }
        bool IsVisible() const { return m_Show; }

    private:
        static constexpr int kPerfHistory = 120;

        Editor* m_Owner = nullptr;
        Boom::AppContext* m_Ctx = nullptr;

        bool   m_Show = true;
        float  m_FpsHistory[kPerfHistory]{};
        int    m_FpsWriteIdx = 0;
    };

} // namespace EditorUI
