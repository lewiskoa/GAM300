#include "Core.h"
#include "AppWindow.h"

namespace Boom {
	BOOM_INLINE AppWindow::AppWindow(EventDispatcher* disp, int32_t w, int32_t h, char const* windowTitle)
		: width{ w }
		, height{ h }
		, refreshRate{ 144 }
		, title{ windowTitle }
		, isFullscreen{ false }
		, windowPtr{}
		, monitorPtr{}
		, modePtr{}
		, dispatcher{disp}
	{
		if (!glfwInit()) {
			BOOM_FATAL("AppWindow::Init() - glfwInit() failed.");
			std::exit(EXIT_FAILURE);
		}
		glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_TRUE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		monitorPtr = glfwGetPrimaryMonitor();
		modePtr = glfwGetVideoMode(monitorPtr);
		glfwWindowHint(GLFW_REFRESH_RATE, modePtr->refreshRate);
		glfwWindowHint(GLFW_GREEN_BITS, modePtr->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, modePtr->blueBits);
		glfwWindowHint(GLFW_RED_BITS, modePtr->redBits);
		glfwWindowHint(GLFW_SAMPLES, 4);

		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		glfwWindowHint(GLFW_DEPTH_BITS, 24);

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
		SetupCallbacks(windowPtr);
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

	BOOM_INLINE void AppWindow::SetupCallbacks(GLFWwindow* win) {
		glfwSetErrorCallback(AppWindow::OnError);
		glfwSetWindowMaximizeCallback(win, AppWindow::OnMaximized);
		glfwSetFramebufferSizeCallback(win, AppWindow::OnResize);
		glfwSetWindowIconifyCallback(win, AppWindow::OnIconify);
		glfwSetWindowCloseCallback(win, AppWindow::OnClose);
		glfwSetWindowFocusCallback(win, AppWindow::OnFocus);

		glfwSetScrollCallback(win, AppWindow::OnWheel);
		glfwSetMouseButtonCallback(win, AppWindow::OnMouse);
		glfwSetCursorPosCallback(win, AppWindow::OnMotion);
		glfwSetKeyCallback(win, AppWindow::OnKey);
	}

	BOOM_INLINE void AppWindow::OnError(int32_t errorNo, char const* description) {
		BOOM_ERROR("[GLFW]: [{}] {}", errorNo, description);
	}

	BOOM_INLINE void AppWindow::OnMaximized(GLFWwindow* win, int32_t action) {
		AppWindow* self{ GetUserData(win) };
		if (action) {
			self->isFullscreen = true;
			//GetUserData(win)->dispatcher->PostEvent<WindowMaximizeEvent>();
			self->dispatcher->PostTask([&self] {
					glfwSetWindowMonitor(
						self->windowPtr, 
						self->monitorPtr, 
						0, 
						0, 
						self->modePtr->width, 
						self->modePtr->height, 
						self->modePtr->refreshRate
					);
				}
			);
		}
		else {
			self->isFullscreen = false;
			//enable input + unpause system + change to non fullscreen or not + etc...
			//GetUserData(win)->dispatcher->PostEvent<WindowRestoreEvent>();
		}
	}

	BOOM_INLINE void AppWindow::OnIconify(GLFWwindow* win, int32_t action) {
		(void)win;
		//AppWindow* self{ GetUserData(win) };
		if (action) {
			//disable input + pause system + etc...
			//self->dispatcher->PostEvent<WindowIconifyEvent>();
		}
		else {
			//enable input + unpause system + check fullscreen or not + etc...
			//self->dispatcher->PostEvent<WindowRestoreEvent>();
		}
	}
	BOOM_INLINE void AppWindow::OnClose(GLFWwindow* win) {
		(void)win;
		//AppWindow* self{ GetUserData(win) };
		//self->dispatcher->PostEvent<WindowCloseEvent>();
		//this event struct can be implemented to contain other needed system logic upon closing window
		//GetUserData(win)->dispatcher->PostEvent<WindowCloseEvent>();
	}
	BOOM_INLINE void AppWindow::OnResize(GLFWwindow* win, int32_t w, int32_t h) {
		(void)win; (void)w; (void)h;
		//GetUserData(win)->dispatcher->PostEvent<WindowResizeEvent>(w, h);
	}
	BOOM_INLINE void AppWindow::OnFocus(GLFWwindow* win, int32_t focused) {
		(void)win; (void)focused;
		//implemented due to gam250 requirement of pausing the game when unfocused
		//assuming might carry over
	}

	BOOM_INLINE void AppWindow::OnWheel(GLFWwindow* win, double x, double y) {
		(void)win; (void)x; (void)y;
		/*
		GetUserData(win)->dispatcher->PostEvent<MouseWheelEvent>(x, y);
		*/
	}
	BOOM_INLINE void AppWindow::OnMouse(GLFWwindow* win, int32_t button, int32_t action, int32_t) {
		(void)win;
		//AppWindow* self{ GetUserData(win) };

		if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST) {
			switch (action) {
			case GLFW_RELEASE:
				//self->dispatcher->PostEvent<MouseReleaseEvent>(button);
				//self->inputs.Mouse.reset(button);
				break;

			case GLFW_PRESS:
				//self->dispatcher->PostEvent<MouseDownEvent>(button);
				//self->inputs.Mouse.set(button);
				break;
			}
			return;
		}
		BOOM_ERROR("AppWindow::OnMouse() invalid keycode: [{}]", button);
	}
	BOOM_INLINE void AppWindow::OnMotion(GLFWwindow* win, double x, double y) {
		/*
		//AppWindow* self{ GetUserData(win) };
		//self->dispatcher->PostEvent<MouseMotionEvent>(x, y);
		if (self->inputs.Mouse.test(GLFW_MOUSE_BUTTON_LEFT)) {
			//directional distance between new pos and old pos
			self->dispatcher->PostEvent<MouseDragEvent>(self->inputs.MouseX - x, self->inputs.MouseY - y);
		}
		self->inputs.MouseX = x;
		self->inputs.MouseY = y;
		*/
		(void)win;  (void)x; (void)y; //remove when added in
	}
	BOOM_INLINE void AppWindow::OnKey(GLFWwindow* win, int32_t key, int32_t, int32_t action, int32_t) {
		//AppWindow* self{ GetUserData(win) };
		
		//temporary force close...
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			//self->dispatcher->PostEvent<WindowCloseEvent>();
			glfwSetWindowShouldClose(win, GLFW_TRUE);
			return;
		}

		if (key >= 0 && key <= GLFW_KEY_LAST) {
			switch (action) {
			case GLFW_RELEASE:
				//self->dispatcher->PostEvent<KeyReleaseEvent>(key);
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

	[[nodiscard]] int32_t& AppWindow::Width() noexcept {
		return width;
	}
	[[nodiscard]] int32_t& AppWindow::Height() noexcept {
		return height;
	}
	GLFWwindow* AppWindow::Window() noexcept {
		return windowPtr;
	}
	BOOM_INLINE bool AppWindow::PollEvents() {
		glfwPollEvents();
		dispatcher->PollEvents();
		glfwSwapBuffers(windowPtr);
		return !glfwWindowShouldClose(windowPtr);
	}
	BOOM_INLINE bool AppWindow::IsKey(int32_t key) const {
		if (key >= 0 && key <= GLFW_KEY_LAST)
			return true; //inputs.Keys.test(key);
		return false;
	}
	BOOM_INLINE bool AppWindow::IsMouse(int32_t button) const {
		if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
			return true; //inputs.Mouse.test(button);
		return false;
	}
	BOOM_INLINE int AppWindow::IsExit() const {
		return glfwWindowShouldClose(windowPtr);
	}
}