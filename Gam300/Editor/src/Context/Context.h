#pragma once
#include "Widgets.h"

struct GuiContext : AppInterface
{
	BOOM_INLINE virtual ~GuiContext()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		glfwTerminate();
	}

	BOOM_INLINE virtual void OnGuiStart() {}
	BOOM_INLINE virtual void OnGuiFrame() {}

	BOOM_INLINE virtual void OnStart() override final
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigWindowsMoveFromTitleBarOnly = true;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	}

};