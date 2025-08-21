#include "Core.h"
#include "AppWindow.h"

namespace Boom {
	GLFWwindow* AppWindow::windowPtr{};
	GLFWmonitor* AppWindow::monitorPtr{};
	GLFWvidmode const* AppWindow::modePtr{};

	AppWindow::AppWindow(int32_t w, int32_t h, char const* windowTitle)
		: width{ w }
		, height{ h }
		, refreshRate{ 144 }
		, title{ windowTitle }
		, isFullscreen{ false }
	{
		if (!glfwInit()) {
			BOOM_FATAL("AppWindow::Init() - glfwInit() failed.");
			std::exit(EXIT_FAILURE);
		}

		monitorPtr = glfwGetPrimaryMonitor();
		modePtr = glfwGetVideoMode(monitorPtr);
		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		glfwWindowHint(GLFW_DEPTH_BITS, 24);
		glfwWindowHint(GLFW_REFRESH_RATE, modePtr->refreshRate);
		glfwWindowHint(GLFW_GREEN_BITS, modePtr->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, modePtr->blueBits);
		glfwWindowHint(GLFW_RED_BITS, modePtr->redBits);
		glfwWindowHint(GLFW_SAMPLES, 4);

		glfwWindowHint(GLFW_MAXIMIZED, GL_FALSE);
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

		windowPtr = glfwCreateWindow(width, height, title, NULL, NULL);
		if (windowPtr == nullptr) {
			BOOM_FATAL("AppWindow::Init() - failed to init app window.");
			std::exit(EXIT_FAILURE);
		}
		glfwMakeContextCurrent(windowPtr);

		//user data
		glfwSetWindowUserPointer(windowPtr, this);
		//enable a-vsync
		glfwSwapInterval(-1);
		SetupCallbacks();
	}
	BOOM_INLINE AppWindow::~AppWindow() {
		glfwDestroyWindow(windowPtr);
		glfwTerminate();
	}
	BOOM_INLINE void AppWindow::OnStart() {

	}
	BOOM_INLINE void AppWindow::OnUpdate() {
		glfwSwapBuffers(Window());
		glfwPollEvents();
	}

	void AppWindow::SetupCallbacks() {

		glfwSetErrorCallback(AppWindow::OnError);
		glfwSetWindowMaximizeCallback(windowPtr, AppWindow::OnMaximized);
		glfwSetFramebufferSizeCallback(windowPtr, AppWindow::OnResize);
		glfwSetWindowIconifyCallback(windowPtr, AppWindow::OnIconify);
		glfwSetWindowCloseCallback(windowPtr, AppWindow::OnClose);
		glfwSetWindowFocusCallback(windowPtr, AppWindow::OnFocus);

		glfwSetScrollCallback(windowPtr, AppWindow::OnWheel);
		glfwSetMouseButtonCallback(windowPtr, AppWindow::OnMouse);
		glfwSetCursorPosCallback(windowPtr, AppWindow::OnMotion);
		glfwSetKeyCallback(windowPtr, AppWindow::OnKey);
	}

	BOOM_INLINE void AppWindow::OnError(int32_t errorNo, char const* description) {
		BOOM_ERROR("[GLFW]: [{}] {}", errorNo, description);
	}

	BOOM_INLINE void AppWindow::OnMaximized(GLFWwindow* win, int32_t action) {
		(void)win;
		if (action) {
			//GetUserData(win)->isFullscreen;
		}
	}

	BOOM_INLINE void AppWindow::OnIconify(GLFWwindow* win, int32_t action) {
		(void)win; (void)action;
	}
	BOOM_INLINE void AppWindow::OnClose(GLFWwindow* win) {
		(void)win;
	}
	BOOM_INLINE void AppWindow::OnResize(GLFWwindow* win, int32_t w, int32_t h) {
		(void)win; (void)w; (void)h;
	}
	BOOM_INLINE void AppWindow::OnFocus(GLFWwindow* win, int32_t focused) {
		(void)win; (void)focused;
	}

	BOOM_INLINE void AppWindow::OnWheel(GLFWwindow* win, double x, double y) {
		(void)win; (void)x; (void)y;
	}
	BOOM_INLINE void AppWindow::OnMouse(GLFWwindow* win, int32_t button, int32_t action, int32_t) {
		(void)win;
		//AppWindow* self{ GetUserData(win) };

		if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST) {
			switch (action) {
			case GLFW_RELEASE:
				//mouse release event
				break;

			case GLFW_PRESS:
				//...
				break;
			}
			return;
		}
		BOOM_ERROR("AppWindow::OnMouse() invalid keycode: [{}]", button);
	}
	BOOM_INLINE void AppWindow::OnMotion(GLFWwindow* win, double x, double y) {
		//AppWindow* self{ GetUserData(win) };
		//ADD HERE: mouse movement post_event(set in this function)
		//GLFW_MOUSE_BUTTON_LEFT;
		(void)win;  (void)x; (void)y; //remove when added in
	}
	BOOM_INLINE void AppWindow::OnKey(GLFWwindow* win, int32_t key, int32_t scancode, int32_t action, int32_t) {
		(void)scancode; (void)win;
		//AppWindow* self{ GetUserData(win) };
		
		//temporary force close...
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(windowPtr, GLFW_TRUE);
			return;
		}

		if (key >= 0 && key <= GLFW_KEY_LAST) {
			switch (action) {
			case GLFW_RELEASE:
				//key release event
				break;

			case GLFW_PRESS:
				//...
				break;

			case GLFW_REPEAT:
				break;
			}
			return;
		}
		
		BOOM_ERROR("AppWindow::OnKey() invalid keycode: [{}]", key);
	}

	BOOM_INLINE AppWindow* AppWindow::GetUserData(GLFWwindow* window) {
		return static_cast<AppWindow*>(glfwGetWindowUserPointer(window));
	}

	void AppWindow::ToggleFullscreen(bool firstRun) {
		if (firstRun) {
			//do deserialization stuff...
		}
		else
			isFullscreen = !isFullscreen;

		if (isFullscreen)
			glfwSetWindowMonitor(windowPtr, monitorPtr, 0, 0, modePtr->width, modePtr->height, modePtr->refreshRate);
		else
			glfwSetWindowMonitor(windowPtr, NULL, 100, 100, width, height, refreshRate);
	}

	[[nodiscard]] int32_t& AppWindow::Width() noexcept {
		return width;
	}
	[[nodiscard]] int32_t& AppWindow::Height() noexcept {
		return height;
	}
	GLFWwindow* AppWindow::Window() noexcept {
		return windowPtr;
	}
	[[nodiscard]] int AppWindow::IsExit() const {
		return glfwWindowShouldClose(windowPtr);
	}
}