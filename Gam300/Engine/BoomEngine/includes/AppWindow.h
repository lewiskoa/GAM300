#pragma once

#include "Core.h" //used for variables and error handling
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
class Test {
public:
	Test();
	Test(Test const&) = delete;
	BOOM_INLINE void Init();
	~Test();
};

namespace Boom {

	class AppWindow {
	public:
		AppWindow();
		AppWindow(AppWindow const&) = delete;

		BOOM_INLINE void Init();
		void Update(float dt);
		~AppWindow();

	private: //GLFW callbacks
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

	public:
		void ToggleFullscreen(bool firstRun = false);

		[[nodiscard]] int32_t& Width() noexcept;
		[[nodiscard]] int32_t& Height() noexcept;

		void PrintSpecs();

	private:
		int32_t width;
		int32_t height;
		int32_t refreshRate;
		std::string title;
		bool isFullscreen;

		//glfw window context
		static GLFWwindow* windowPtr;
		static GLFWmonitor* monitorPtr;
		static GLFWvidmode const* modePtr;
	};
}