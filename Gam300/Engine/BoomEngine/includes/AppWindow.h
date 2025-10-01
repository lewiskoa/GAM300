#pragma once
#include "Core.h" //used for variables and error handling
#include "common/Events.h"
#include "GlobalConstants.h"
#include <iostream>
namespace Boom {
	struct AppWindow {
	public:
		AppWindow() = delete;
		AppWindow(AppWindow const&) = delete;

		//example for initializing with reference:
		//std::unique_ptr<AppWindow> w = std::make_unique<AppWindow>(&Dispatcher, 1900, 800, "BOOM");
		BOOM_INLINE AppWindow(EventDispatcher* disp, int32_t w, int32_t h, char const* windowTitle)
			: width{ w }
			, height{ h }
			, refreshRate{ 144 }
			, isFullscreen{ false }
			, monitorPtr{}
			, modePtr{}
			, windowPtr{}
			, dispatcher{ disp }
		{
			if (!glfwInit()) {
				BOOM_FATAL("AppWindow::Init() - glfwInit() failed.");
				std::exit(EXIT_FAILURE);
			}
			//glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_TRUE);
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

			windowPtr = std::shared_ptr<GLFWwindow>(
				glfwCreateWindow(width, height, windowTitle, NULL, NULL),
				glfwDestroyWindow
			);
			if (windowPtr.get() == nullptr) {
				BOOM_FATAL("AppWindow::Init() - failed to init app window.");
				std::exit(EXIT_FAILURE);
			}

			// ADD THESE LINES:
			BOOM_INFO("AppWindow - Initial window size: {}x{}", width, height);
			glfwShowWindow(windowPtr.get());  // Make sure window is visible
			glfwFocusWindow(windowPtr.get()); // Bring to front

			int actualWidth, actualHeight;
			glfwGetWindowSize(windowPtr.get(), &actualWidth, &actualHeight);
			BOOM_INFO("AppWindow - Actual window size after creation: {}x{}", actualWidth, actualHeight);

			glfwMakeContextCurrent(windowPtr.get());
			GLFWwindow* current = glfwGetCurrentContext();

			if (current != windowPtr.get()) {
				BOOM_ERROR("Failed to make window context current in constructor!");
			}

			//user data
			glfwSetWindowUserPointer(windowPtr.get(), this);
			//enable a-vsync
			glfwSwapInterval(-1);
			SetupCallbacks(windowPtr.get());

			//initial window color
			std::apply(glClearColor, CONSTANTS::DEFAULT_BACKGROUND_COLOR);

			glfwGetCursorPos(windowPtr.get(), &prevMousePos.x, &prevMousePos.y);
		}
		BOOM_INLINE ~AppWindow() {}

	private: //GLFW callbacks
		BOOM_INLINE static void SetupCallbacks(GLFWwindow* win) {
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
		BOOM_INLINE static void OnError(int32_t errorNo, char const* description) {
			BOOM_ERROR("[GLFW]: [{}] {}", errorNo, description);
		}
		BOOM_INLINE static void OnMaximized(GLFWwindow* win, int32_t action) {
			AppWindow* self{ GetUserData(win) };
			if (action) {
				self->isFullscreen = true;
				//GetUserData(win)->dispatcher->PostEvent<WindowMaximizeEvent>();
				/*
				self->dispatcher->PostTask([&self] {
					glfwSetWindowMonitor(
						self->windowPtr.get(),
						self->monitorPtr,
						0,
						0,
						self->modePtr->width,
						self->modePtr->height,
						self->modePtr->refreshRate
					);
					}
				);*/
			}
			else {
				self->isFullscreen = false;
				//enable input + unpause system + change to non fullscreen or not + etc...
				//GetUserData(win)->dispatcher->PostEvent<WindowRestoreEvent>();
			}
		}
		BOOM_INLINE static void OnResize(GLFWwindow* win, int32_t w, int32_t h) {
			(void)win; (void)w; (void)h;
			GetUserData(win)->dispatcher->PostEvent<WindowResizeEvent>(w, h);
		}
		BOOM_INLINE static void OnIconify(GLFWwindow* win, int32_t action) {
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
		BOOM_INLINE static void OnClose(GLFWwindow* win) {
			(void)win;
			//AppWindow* self{ GetUserData(win) };
			//self->dispatcher->PostEvent<WindowCloseEvent>();
			//this event struct can be implemented to contain other needed system logic upon closing window
			//GetUserData(win)->dispatcher->PostEvent<WindowCloseEvent>();
		}
		BOOM_INLINE static void OnFocus(GLFWwindow* win, int32_t focused) {
			(void)win; (void)focused;
			//implemented due to gam250 requirement of pausing the game when unfocused
			//assuming might carry over
		}

		BOOM_INLINE static void OnWheel(GLFWwindow* win, double x, double y) {
			(void)x;
			/*
			GetUserData(win)->dispatcher->PostEvent<MouseWheelEvent>(x, y);
			*/
			AppWindow* self{ GetUserData(win) };
			self->camMoveDir.z -= (float)y * CONSTANTS::CAM_PAN_SPEED;;
		}
		BOOM_INLINE static void OnMouse(GLFWwindow* win, int32_t button, int32_t action, int32_t) {
			AppWindow* self{ GetUserData(win) };
			
			if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST) {
				switch (action) {
				case GLFW_RELEASE:
					//self->dispatcher->PostEvent<MouseReleaseEvent>(button);
					//self->inputs.Mouse.reset(button);

					if (button == GLFW_MOUSE_BUTTON_RIGHT) {
						self->isRightClickDown = false;
					}
					if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
						self->isMiddleClickDown = false;
					}
					break;

				case GLFW_PRESS:
					//self->dispatcher->PostEvent<MouseDownEvent>(button);
					//self->inputs.Mouse.set(button);

					if (button == GLFW_MOUSE_BUTTON_RIGHT) {
						self->isRightClickDown = true;
					}
					if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
						self->isMiddleClickDown = true;
					}
					break;
				}
				return;
			}
			BOOM_ERROR("AppWindow::OnMouse() invalid keycode: [{}]", button);
		}
		BOOM_INLINE static void OnMotion(GLFWwindow* win, double x, double y) {
			AppWindow* self{ GetUserData(win) };
			
			//panning camera position
			if (self->isMiddleClickDown) {
				glm::vec2 pan{ glm::dvec2{
					(self->prevMousePos.x - x) / (double)self->width,
					(y - self->prevMousePos.y) / (double)self->height} };
				//TODO: these magic numbers [9,11] should be calculated accordingly as:
				// (self->width / imgui_vp_resolution.x), (self->height / imgui_vp_resolution.y)
				// please convert when done so.
				self->camMoveDir.y += pan.y * 9.f; 
				self->camMoveDir.x += pan.x * 11.f;
			}
			//camera yaw and pitch
			else if (self->isRightClickDown) {
				self->camRot.x += (float)(self->prevMousePos.y - y) * CONSTANTS::CAM_PAN_SPEED;
				self->camRot.y += (float)(self->prevMousePos.x - x) * CONSTANTS::CAM_PAN_SPEED;
			}
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
			self->prevMousePos = { x, y };
		}
		BOOM_INLINE static void OnKey(GLFWwindow* win, int32_t key, int32_t, int32_t action, int32_t) {
			AppWindow* self{ GetUserData(win) };

			//temporary force close...
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
				//self->dispatcher->PostEvent<WindowCloseEvent>();
				glfwSetWindowShouldClose(win, GLFW_TRUE);
				return;
			}
			//camera strafing and hovering
			if (self->isRightClickDown) {
				if (action == GLFW_PRESS || action == GLFW_REPEAT) {
					if (key == GLFW_KEY_W) {
						self->camMoveDir.y += CONSTANTS::CAM_PAN_SPEED;
					}
					if (key == GLFW_KEY_S) {
						self->camMoveDir.y -= CONSTANTS::CAM_PAN_SPEED;
					}
					if (key == GLFW_KEY_A) {
						self->camMoveDir.x -= CONSTANTS::CAM_PAN_SPEED;
					}
					if (key == GLFW_KEY_D) {
						self->camMoveDir.x += CONSTANTS::CAM_PAN_SPEED;
					}
				}
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

		//to be called explicitly to reference window from application
		//example: GetUserData(win)->isFullscreen 
		BOOM_INLINE static AppWindow* GetUserData(GLFWwindow* window) {
			return static_cast<AppWindow*>(glfwGetWindowUserPointer(window));
		}

	public:
		BOOM_INLINE void SetWindowTitle(std::string const& title) {
			glfwSetWindowTitle(windowPtr.get(), title.c_str());
		}

		[[nodiscard]] BOOM_INLINE int32_t& Width() noexcept {
			return width;
		}
		[[nodiscard]] BOOM_INLINE int32_t& Height() noexcept {
			return height;
		} 
		[[nodiscard]] BOOM_INLINE std::shared_ptr<GLFWwindow> Handle() const {
			return windowPtr;
		}
		
		BOOM_INLINE bool PollEvents() {
			glfwPollEvents();
			dispatcher->PollEvents();
			glfwSwapBuffers(windowPtr.get());
			return !glfwWindowShouldClose(windowPtr.get());
		}
		BOOM_INLINE bool IsKey(int32_t key) const {
			if (key >= 0 && key <= GLFW_KEY_LAST)
				return true; //inputs.Keys.test(key);
			return false;
		}
		BOOM_INLINE bool IsMouse(int32_t button) const {
			if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
				return true; //inputs.Mouse.test(button);
			return false;
		}
		BOOM_INLINE int IsExit() const {
			return glfwWindowShouldClose(windowPtr.get());
		}
		
	private:
		int32_t width;
		int32_t height;
		int32_t refreshRate;
		bool isFullscreen;

		//these are kept raw as they are lightweight and non-owning
		GLFWmonitor* monitorPtr;
		GLFWvidmode const* modePtr;
		std::shared_ptr<GLFWwindow> windowPtr;
		EventDispatcher* dispatcher;
		//WindowInputs inputs;

	public: //usage for basic glew input to move camera in editor
		bool isRightClickDown{};
		bool isMiddleClickDown{};
		//x - strafe right/left
		//y - hover  up/down
		//z - zoom   front/back
		//need to be manually reset after user used to move cam
		glm::vec3 camMoveDir{};
		glm::dvec2 prevMousePos{};
		//around x/y
		//need to be manually reset after user used to rotate cam
		glm::vec2 camRot{};
	};
}
