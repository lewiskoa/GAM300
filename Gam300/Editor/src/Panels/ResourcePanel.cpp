#include "Panels/ResourcePanel.h"
#include "Editor.h"
#include "Context/Context.h"
#include "Context/DebugHelpers.h"
#include "Vendors/imgui/imgui.h"

#ifndef ICON_FA_IMAGE
#define ICON_FA_IMAGE ""
#endif

namespace EditorUI {

    ResourcePanel::ResourcePanel(Editor* owner)
        : m_Owner(owner)
    {
        DEBUG_DLL_BOUNDARY("ResourcePanel::Ctor");
        if (!m_Owner) { BOOM_ERROR("ResourcePanel - null owner"); return; }
        m_Ctx = m_Owner->GetContext();
        DEBUG_POINTER(m_Ctx, "AppContext");
    }

    void ResourcePanel::OnShow()
    {
        if (!ImGui::Begin("Resources")) { ImGui::End(); return; }

        ImGui::Text("Asset tile size: %.0f", ASSET_SIZE);

        ImGui::BeginGroup();
        if (m_Icon) {
            ImGui::Image(m_Icon, ImVec2(ASSET_SIZE, ASSET_SIZE));
        }
        else {
            const char* label = (ICON_FA_IMAGE[0] ? ICON_FA_IMAGE : " ");
            ImGui::Button(label, ImVec2(ASSET_SIZE, ASSET_SIZE));
        }
        ImGui::TextUnformatted("Example.asset");
        ImGui::EndGroup();

        ImGui::End();
    }

} // namespace EditorUI
