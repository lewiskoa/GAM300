#include "PropertiesImgui.h"

namespace EditorUI {
     void DrawPropertiesUI(const xproperty::type::object* pObj, void* pInstance)
    {
        xproperty::settings::context ctx;

        for (auto& member : pObj->m_Members) {
            DrawPropertyMember(member, pInstance, ctx);
        }
    }

  void DrawPropertyMember(const xproperty::type::members& member, void* pInstance, xproperty::settings::context& ctx)
    {
        ImGui::PushID(member.m_pName);

        if (std::holds_alternative<xproperty::type::members::var>(member.m_Variant)) {
            auto& var = std::get<xproperty::type::members::var>(member.m_Variant);

            xproperty::any value;
            var.m_pRead(pInstance, value, var.m_UnregisteredEnumSpan, ctx);

            auto typeGUID = value.getTypeGuid();
            bool changed = false;

            void* pData = &value.m_Data;

            // Label column (Unity-style: label on left, control on right)
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%s", member.m_pName);
            ImGui::SameLine(150); // Fixed label width like Unity
            ImGui::SetNextItemWidth(-1); // Fill remaining width

            if (typeGUID == xproperty::settings::var_type<float>::guid_v) {
                float* pValue = reinterpret_cast<float*>(pData);
                changed = ImGui::DragFloat("##value", pValue, 0.01f);
            }
            else if (typeGUID == xproperty::settings::var_type<glm::vec3>::guid_v) {
                glm::vec3* pValue = reinterpret_cast<glm::vec3*>(pData);
                changed = ImGui::DragFloat3("##value", &pValue->x, 0.01f);
            }
            else if (typeGUID == xproperty::settings::var_type<int32_t>::guid_v) {
                int32_t* pValue = reinterpret_cast<int32_t*>(pData);
                changed = ImGui::DragInt("##value", pValue);
            }
            else if (typeGUID == xproperty::settings::var_type<uint64_t>::guid_v) {
                uint64_t* pValue = reinterpret_cast<uint64_t*>(pData);
                changed = ImGui::InputScalar("##value", ImGuiDataType_U64, pValue);
            }
            else if (typeGUID == xproperty::settings::var_type<std::string>::guid_v) {
                std::string* pValue = reinterpret_cast<std::string*>(pData);
                char buffer[256];
                strncpy_s(buffer, pValue->c_str(), sizeof(buffer));
                if (ImGui::InputText("##value", buffer, sizeof(buffer))) {
                    *pValue = std::string(buffer);
                    changed = true;
                }
            }
            else if (value.isEnum()) {
                const auto& enumSpan = value.getEnumSpan();
                const char* currentName = value.getEnumString();

                if (ImGui::BeginCombo("##value", currentName)) {
                    for (const auto& enumItem : enumSpan) {
                        bool selected = (enumItem.m_Value == value.getEnumValue());
                        if (ImGui::Selectable(enumItem.m_pName, selected)) {
                            xproperty::any newValue;
                            newValue.set<std::string>(enumItem.m_pName);
                            var.m_pWrite(pInstance, newValue, var.m_UnregisteredEnumSpan, ctx);
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            else {
                ImGui::TextDisabled("<unsupported>");
            }

            if (changed && !member.m_bConst && var.m_pWrite) {
                var.m_pWrite(pInstance, value, var.m_UnregisteredEnumSpan, ctx);
            }
        }
        else if (std::holds_alternative<xproperty::type::members::props>(member.m_Variant)) {
            auto& props = std::get<xproperty::type::members::props>(member.m_Variant);
            auto [pChild, pChildObj] = props.m_pCast(pInstance, ctx);

            if (pChild && pChildObj) {
                // Nested object with subtle indentation
                if (ImGui::TreeNodeEx(member.m_pName, ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Indent(8.0f);
                    for (auto& childMember : pChildObj->m_Members) {
                        DrawPropertyMember(childMember, pChild, ctx);
                    }
                    ImGui::Unindent(8.0f);
                    ImGui::TreePop();
                }
            }
        }

        ImGui::PopID();
    }
}