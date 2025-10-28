#pragma once
#include "Core.h" //used for variables and error handling
#include "common/Events.h"
#include "GlobalConstants.h"

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
			glfwSwapInterval(0);
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
			}
			else {
				self->isFullscreen = false;
				//enable input + unpause system + change to non fullscreen or not + etc...
				//GetUserData(win)->dispatcher->PostEvent<WindowRestoreEvent>();
			}
		}
		BOOM_INLINE static void OnResize(GLFWwindow* win, int32_t w, int32_t h) {
			AppWindow* self{ GetUserData(win) };
			if (!self->isEditor) {
				self->dispatcher->PostEvent<WindowResizeEvent>(w, h);
			}
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
			if (self->isRightClickDown) {
				float sum{ (float)y * 0.01f };
				if (self->isShiftDown) {
					sum *= 10.f;
				}
				self->camMoveMultiplier = glm::clamp(self->camMoveMultiplier + sum, 0.01f, 100.f);
			}
			else if (self->IsMouseInCameraRegion(win)) {
				self->SetFOV(self->camFOV - (float)y);
			}
		}
		BOOM_INLINE static void OnMouse(GLFWwindow* win, int32_t button, int32_t action, int32_t) {
			AppWindow* self{ GetUserData(win) };

			if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST) {
				switch (action) {
				case GLFW_RELEASE:
					if (button == GLFW_MOUSE_BUTTON_RIGHT)  self->isRightClickDown = false;
					if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
						self->isMiddleClickDown = false;
						self->camMoveDir = {};
					}
					break;

				case GLFW_PRESS:
					if (button == GLFW_MOUSE_BUTTON_RIGHT) {
						// Only enable camera look if we're allowed AND the cursor is inside the scene viewport
						if (self->camInputEnabled && self->IsMouseInCameraRegion(win)) {
							self->isRightClickDown = true;
						}
						else {
							self->isRightClickDown = false; // hard block
						}
					}
					if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
						// Middle-button panning can be likewise gated (optional)
						self->isMiddleClickDown = self->camInputEnabled && self->IsMouseInCameraRegion(win);
					}
					break;
				}
				return;
			}
			BOOM_ERROR("AppWindow::OnMouse() invalid keycode: [{}]", button);
		}

		BOOM_INLINE static void OnMotion(GLFWwindow* win, double x, double y) {
			AppWindow* self{ GetUserData(win) };

			const bool inRegion = self->IsMouseInCameraRegion(win);

			// camera yaw/pitch
			if (self->isRightClickDown && self->camInputEnabled && inRegion) {
				self->camRot.x = (float)(self->prevMousePos.y - y) * CONSTANTS::CAM_PAN_SPEED;
				self->camRot.y = (float)(self->prevMousePos.x - x) * CONSTANTS::CAM_PAN_SPEED;
			}

			else if (self->isMiddleClickDown && self->camInputEnabled && inRegion) {
				glm::vec2 pan{ glm::dvec2{
					(self->prevMousePos.x - x) / (double)self->width,
					(y - self->prevMousePos.y) / (double)self->height } };
				self->camMoveDir.y = pan.y * self->camFOV * 0.1f;
				self->camMoveDir.x = pan.x * self->camFOV * 0.09f;
			}

			self->prevMousePos = { x, y };
		}

		BOOM_INLINE static void OnKey(GLFWwindow* win, int32_t key, int32_t, int32_t action, int32_t) {
			AppWindow* self{ GetUserData(win) };

			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
				glfwSetWindowShouldClose(win, GLFW_TRUE);
				return;
			}

			// Ignore weird scancodes
			if (key < 0 || key > GLFW_KEY_LAST) return;

			// Update state
			const bool down = (action != GLFW_RELEASE);
			self->inputs.Keys.set(static_cast<size_t>(key), down);

			const bool run = self->inputs.Keys.test(GLFW_KEY_LEFT_SHIFT);

			// Axis from state (pos - neg) -> {-1,0,1}
			auto axis = [&](int pos, int neg) -> float {
				return float(self->inputs.Keys.test(pos)) - float(self->inputs.Keys.test(neg));
				};

			// Build movement vector from current state 
			const float base = CONSTANTS::CAM_PAN_SPEED * self->camMoveMultiplier;
			const float spd = base * (run ? CONSTANTS::CAM_RUN_MULTIPLIER : 1.f);

			glm::vec3 dir{
				axis(GLFW_KEY_D, GLFW_KEY_A),   // X: +D, -A
				axis(GLFW_KEY_E, GLFW_KEY_Q),   // Y: +E, -Q
				axis(GLFW_KEY_S, GLFW_KEY_W)    // Z: +S, -W  
			};

			// Optional: normalize diagonal so WASD diagonals aren’t faster
			if (glm::length2(dir) > 1e-6f) {
				dir = glm::normalize(dir);
			}

			self->camMoveDir = dir * spd;

			// (Optional) keep your events for other systems
			switch (action) {
			case GLFW_PRESS:  self->dispatcher->PostEvent<KeyPressEvent>(key);  break;
			case GLFW_RELEASE:self->dispatcher->PostEvent<KeyReleaseEvent>(key); break;
			case GLFW_REPEAT: self->dispatcher->PostEvent<KeyRepeatEvent>(key); break;
			}
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
		BOOM_INLINE std::shared_ptr<GLFWwindow> Handle() const {
			return windowPtr;
		}

		BOOM_INLINE bool PollEvents() {
			glfwPollEvents();
			dispatcher->PollEvents();
			glfwSwapBuffers(windowPtr.get());
			return !glfwWindowShouldClose(windowPtr.get());
		}
		BOOM_INLINE bool IsKey(int32_t key) const {
			return key >= 0 && key <= GLFW_KEY_LAST
				&& inputs.Keys.test(static_cast<size_t>(key));
		}
		BOOM_INLINE bool IsMouse(int32_t button) const {
			if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
				return true; //inputs.Mouse.test(button);
			return false;
		}
		BOOM_INLINE int IsExit() const {
			return glfwWindowShouldClose(windowPtr.get());
		}
		BOOM_INLINE void SetFOV(float fov) {
			camFOV = glm::clamp(fov, CONSTANTS::MIN_FOV, CONSTANTS::MAX_FOV);
			BOOM_DEBUG("fov:{}", camFOV);
		}
		BOOM_INLINE void SetCameraInputRegion(double x, double y, double w, double h, bool enabled) {
			camRegionX = x; camRegionY = y; camRegionW = w; camRegionH = h; camInputEnabled = enabled;
		}

		BOOM_INLINE bool IsMouseInCameraRegion(GLFWwindow* win) const {
			double mx, my; glfwGetCursorPos(win, &mx, &my); // client-area coords
			return (mx >= camRegionX && mx <= camRegionX + camRegionW &&
				my >= camRegionY && my <= camRegionY + camRegionH);
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
		WindowInputs inputs;

	public: //usage for basic glew input to move camera in editor
		bool isRightClickDown{};
		bool isMiddleClickDown{};
		bool isShiftDown{};
		//x - strafe right/left
		//y - hover  up/down
		//z - zoom   front/back
		glm::vec3 camMoveDir{};
		glm::dvec2 prevMousePos{};
		//around x/y
		glm::vec2 camRot{};
		float camFOV{ CONSTANTS::MIN_FOV };
		float camMoveMultiplier{ 0.5f };

		// Making sure that the rotation on happens in the viewport
		double camRegionX{ 0.0 }, camRegionY{ 0.0 }, camRegionW{ 0.0 }, camRegionH{ 0.0 };
		bool   camInputEnabled{ false };
		bool isEditor{}; //needed due to imgui's weird resizing bug
	};
}
