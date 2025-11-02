#pragma once
#include <BoomEngine.h>
#include <Vendors/FA.h>

#define IMGUI_ENABLE_DOCKING
#include <Vendors/imgui/imgui.h>
#include <Vendors/imgui/imgui_internal.h>


using namespace Boom;

#define FONT_FILE "Resources/Fonts/CascadiaCode.ttf"
#define ICON_FONT "Resources/Fonts/fa-solid-900.ttf"
#define SHADER_VERSION "#version 450"

#define REGULAR_FONT_SIZE 17
#define SMALL_FONT_SIZE 15

#define LABEL_SPACING 130
#define MENUBAR_HEIGHT 50

#define ASSET_SPACING 10
#define ASSET_SIZE 90

//helper macros
#define LINE_HEIGHT()		ImGui::GetTextLineHeight()
#define USE_SMALL_FONT()	ImGui::SetCurrentFont(ImGui::GetIO().Fonts->Fonts[1])
#define USE_REGULAR_FONT()	ImGui::SetCurrentFont(ImGui::GetIO().Fonts->Fonts[0])
#define TEXT_HEIGHT()		ImGui::GetTextLineHeight() + (2.0f * ImGui::GetStyle().FramePadding.y)

