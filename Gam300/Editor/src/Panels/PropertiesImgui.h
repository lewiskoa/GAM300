#pragma once
#include "Auxiliaries/PropertyAPI.h"
#include "EditorPCH.h"
namespace EditorUI {
    void DrawPropertiesUI(const xproperty::type::object* pObj, void* pInstance);
    void DrawPropertyMember(const xproperty::type::members& member, void* pInstance, xproperty::settings::context& ctx);
}