#pragma once
#include "Core.h" //used for variables and error handling
//#include "Application/Interface.h"

namespace Boom {
	class BOOM_API AppWindow {// : public AppInterface {
	public:
		AppWindow() = delete;
		AppWindow(AppWindow const&) = delete;

		//example for initializing with reference:
		//std::unique_ptr<AppWindow> w = std::make_unique<AppWindow>(&Dispatcher, 1900, 800, "BOOM");
		AppWindow(int32_t w, int32_t h, char const* windowTitle);
	
	public: //interface derive
		BOOM_INLINE void OnStart();// override;
		BOOM_INLINE void OnUpdate();// override;
		BOOM_INLINE ~AppWindow();// override;

	private: //GLFW callbacks

		//TODOS: events system not implemented
		BOOM_INLINE static void SetupCallbacks();
		BOOM_INLINE static void OnError(int32_t errorNo, char const* description);
		BOOM_INLINE static void OnMaximized(GLFWwindow* win, int32_t action);
		BOOM_INLINE static void OnResize(GLFWwindow* win, int32_t w, int32_t h);
		BOOM_INLINE static void OnIconify(GLFWwindow* win, int32_t action);
		BOOM_INLINE static void OnClose(GLFWwindow* win);
		BOOM_INLINE static void OnFocus(GLFWwindow* win, int32_t focused);

		BOOM_INLINE static void OnWheel(GLFWwindow* win, double x, double y);
		BOOM_INLINE static void OnMouse(GLFWwindow* win, int32_t button, int32_t action, int32_t);
		BOOM_INLINE static void OnMotion(GLFWwindow* win, double x, double y);
		BOOM_INLINE static void OnKey(GLFWwindow* win, int32_t key, int32_t scancode, int32_t action, int32_t);

		//to be called explicitly to reference window from application
		//example: GetUserData(win)->isFullscreen 
		BOOM_INLINE static AppWindow* GetUserData(GLFWwindow* window);
	public:
		void ToggleFullscreen(bool firstRun = false);

		[[nodiscard]] int32_t& Width() noexcept;
		[[nodiscard]] int32_t& Height() noexcept;
		[[nodiscard]] static GLFWwindow* Window() noexcept;
		[[nodiscard]] int IsExit() const;

		void PrintSpecs();
	private:
		int32_t width;
		int32_t height;
		int32_t refreshRate;
		char const* title;
		bool isFullscreen;

		//glfw window context
		static GLFWwindow* windowPtr;
		static GLFWmonitor* monitorPtr;
		static GLFWvidmode const* modePtr;
	};
}