#pragma once
// We need to be able to interact with different UI input
//elements such number inputs, text inputs, combos inputs,
//etc.The header file "Inputs.h", provide a definition for all
//these different input types.

#include "Helpers.h"

//starts input field
BOOM_INLINE void BeginInput(const char* label)
{
    ImGui::PushID(label);
    ImGui::Columns(2, NULL, false);
    ImGui::SetColumnWidth(0, LABEL_SPACING);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
}

//ends input field
BOOM_INLINE void EndInput()
{
    ImGui::EndColumns();
    ImGui::PopID();
}

//--------------------------------------------------

//show bool input field

BOOM_INLINE bool InputBool(const char* label, bool* value)
{
    BeginInput(label);
    bool hasChanged = ImGui::Checkbox("##", value);
    EndInput();
    return hasChanged;
}

//show float input field
BOOM_INLINE float InputFloat(const char* label, float* value)
{
    BeginInput(label);
    bool hasChanged = ImGui::InputFloat("##", value);
    EndInput();
    return hasChanged;
}

//show vec3 input field
BOOM_INLINE bool InputVec3(const char* label, glm::vec3* value)
{
    BeginInput(label);
    bool hasChanged = ImGui::InputFloat3("##", &value->x);
    EndInput();
    return hasChanged;
}

//show bool (color) input field
BOOM_INLINE bool InputColor(const char* label, float* value)
{
    BeginInput(label);
    bool hasChanged = ImGui::ColorEdit3("##", value);
    EndInput();
    return hasChanged;
}

//shows button
BOOM_INLINE bool InputButton(const char* label, ImVec2 size = ImVec2(0,0))
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.0f, 1.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 1.0f);
    bool clicked = ImGui::ButtonEx(label, size);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    return clicked;

}

//show combo input field

BOOM_INLINE bool InputCombo(const char* label, const std::vector<const char*>& combos)
{
    BeginInput(label);
    bool hasChanged = false;
    static char preview[64] = "Select";  // Fixed size buffer
    if (ImGui::BeginCombo("##", preview))
    {
        for (auto i = 0; i < combos.size(); i++)
        {
            if (ImGui::Selectable(combos[i], !strcmp(combos[i], preview)))
            {
                strncpy_s(preview, combos[i], sizeof(preview) - 1);  // Safer than strcpy
                preview[sizeof(preview) - 1] = '\0';  // Ensure null termination
                hasChanged = true;
            }
        }
        ImGui::EndCombo();
    }
    EndInput();
    return hasChanged;
}

BOOM_INLINE  bool InputText(const char* label, char* value, const char* hint = nullptr, size_t size = 32)
{
    BeginInput(label);
    bool hasChanged = ImGui::InputTextEx("##", hint, value, (int)size, ImVec2(0, 0), ImGuiInputTextFlags_EnterReturnsTrue);
    EndInput();
    return hasChanged;
}