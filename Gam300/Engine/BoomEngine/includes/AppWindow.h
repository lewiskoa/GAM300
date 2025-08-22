#pragma once
#include "Core.h" //used for variables and error handling
#include "common/Events.h"

namespace Boom {
	struct AppWindow {
	public:
		AppWindow() = delete;
		AppWindow(AppWindow const&) = delete;

		//example for initializing with reference:
		//std::unique_ptr<AppWindow> w = std::make_unique<AppWindow>(&Dispatcher, 1900, 800, "BOOM");
		BOOM_INLINE AppWindow(EventDispatcher* disp, int32_t w, int32_t h, char const* windowTitle);
		BOOM_INLINE ~AppWindow();

	private: //GLFW callbacks
		BOOM_INLINE static void SetupCallbacks(GLFWwindow* win);
		BOOM_INLINE static void OnError(int32_t errorNo, char const* description);
		BOOM_INLINE static void OnMaximized(GLFWwindow* win, int32_t action);
		BOOM_INLINE static void OnResize(GLFWwindow* win, int32_t w, int32_t h);
		BOOM_INLINE static void OnIconify(GLFWwindow* win, int32_t action);
		BOOM_INLINE static void OnClose(GLFWwindow* win);
		BOOM_INLINE static void OnFocus(GLFWwindow* win, int32_t focused);

		BOOM_INLINE static void OnWheel(GLFWwindow* win, double x, double y);
		BOOM_INLINE static void OnMouse(GLFWwindow* win, int32_t button, int32_t action, int32_t);
		BOOM_INLINE static void OnMotion(GLFWwindow* win, double x, double y);
		BOOM_INLINE static void OnKey(GLFWwindow* win, int32_t key, int32_t, int32_t action, int32_t);

		//to be called explicitly to reference window from application
		//example: GetUserData(win)->isFullscreen 
		BOOM_INLINE static AppWindow* GetUserData(GLFWwindow* window);

	public:
		[[nodiscard]] int32_t& Width() noexcept;
		[[nodiscard]] int32_t& Height() noexcept;
		[[nodiscard]] BOOM_INLINE GLFWwindow* Window() noexcept;
		BOOM_INLINE bool PollEvents();
		BOOM_INLINE bool IsKey(int32_t key) const;
		BOOM_INLINE bool IsMouse(int32_t button) const;
		BOOM_INLINE int IsExit() const;

		void PrintSpecs();
	private:
		int32_t width;
		int32_t height;
		int32_t refreshRate;
		char const* title;
		bool isFullscreen;

		//glfw window context
		GLFWwindow* windowPtr;
		GLFWmonitor* monitorPtr;
		GLFWvidmode const* modePtr;
		EventDispatcher* dispatcher;
		//WindowInputs inputs;
	};
}