#pragma once
#include "Vendors/imgui/imgui.h"

namespace Boom { class AppContext; }

namespace EditorUI {

    class Editor;

    class ResourcePanel {
    public:
        explicit ResourcePanel(Editor* owner);
        void OnShow();                 // Editor.cpp calls this
        void Render() { OnShow(); }    // optional wrapper

    private:
        Editor* m_Owner = nullptr; // non-owning
        Boom::AppContext* m_Ctx = nullptr; // cached from Editor
        ImTextureID       m_Icon = {};
    };

} // namespace EditorUI
