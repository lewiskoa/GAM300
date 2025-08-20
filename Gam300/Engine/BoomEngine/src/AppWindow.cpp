#include "Core.h"
#include "AppWindow.h"

namespace {
	//helper functions
	std::string GetGlewString(GLenum name, bool isError = false) {
		if (isError)
			return reinterpret_cast<char const*>(glewGetErrorString(name));
		else
			return reinterpret_cast<char const*>(glewGetString(name));
	}
}

namespace Boom {
	GLFWwindow* AppWindow::windowPtr;
	GLFWmonitor* AppWindow::monitorPtr{};
	GLFWvidmode const* AppWindow::modePtr{};

	AppWindow::AppWindow() :
		width{ 1800 },
		height{ 900 },
		refreshRate{ 144 },
		title{ "hello BOOM" },
		isFullscreen{ false }
	{
	}
	AppWindow::~AppWindow() {
		glfwTerminate();
	}

	void AppWindow::Init() {
		//edit: deserialize window var here
		//owowow//

		if (glfwInit() != GLEW_OK) {
			BOOM_FATAL("AppWindow::Init() - glfwInit() failed.");
			std::exit(EXIT_FAILURE);
		}

		glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_TRUE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		monitorPtr = glfwGetPrimaryMonitor();
		modePtr = glfwGetVideoMode(monitorPtr);
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

		//user data
		glfwSetWindowUserPointer(windowPtr, this);
		SetupCallbacks();
		glfwMakeContextCurrent(windowPtr);
		//enable a-vsync
		glfwSwapInterval(-1);

		//Glew stuff
		GLenum err = glewInit();
		if (GLEW_OK != err) {
			BOOM_FATAL("Unable to initialize GLEW - error: {} abort program.\n", GetGlewString(err, true));
			std::exit(EXIT_FAILURE);
		}
		if (GLEW_VERSION_3_3) {
			BOOM_INFO("Using glew version: {}", GetGlewString(GLEW_VERSION));
			BOOM_INFO("Driver supports OpenGL 3.3");
		}
		else {
			BOOM_WARN("Warning: The driver may lack full compatibility with OpenGL 3.3, potentially limiting access to advanced features.");
		}

		PrintSpecs();
	}

	void AppWindow::Update(float dt) {
		(void)dt;
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
		AppWindow* self{ GetUserData(win) };

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
		AppWindow* self{ GetUserData(win) };
		//ADD HERE: mouse movement post_event(set in this function)
		//GLFW_MOUSE_BUTTON_LEFT;
		(void)x; (void)y; //remove when added in
	}
	BOOM_INLINE void AppWindow::OnKey(GLFWwindow* win, int32_t key, int32_t scancode, int32_t action, int32_t) {
		AppWindow* self{ GetUserData(win) };

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
		return static_cast<AppWindow*>(glfwGetWindowUserPointer(windowPtr));
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

	void AppWindow::PrintSpecs() {
		BOOM_DEBUG("GPU Vendor: {}", GetGlewString(GL_VENDOR));
		BOOM_DEBUG("GPU Renderer: {}", GetGlewString(GL_RENDERER));
		BOOM_DEBUG("GPU Version: {}", GetGlewString(GL_VERSION));
		BOOM_DEBUG("GPU Shader Version: {}", GetGlewString(GL_SHADING_LANGUAGE_VERSION));

		GLint output;
		glGetIntegerv(GL_MAJOR_VERSION, &output);
		BOOM_DEBUG("GL Major Version: {}", output);
		glGetIntegerv(GL_MINOR_VERSION, &output);
		BOOM_DEBUG("GL Minor Version: {}", output);

		GLboolean isDB;
		glGetBooleanv(GL_DOUBLEBUFFER, &isDB);
		if (isDB)
			BOOM_DEBUG("Current OpenGL Context is double-buffered");
		else
			BOOM_DEBUG("Current OpenGL Context is not double-buffered");

		glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &output);
		BOOM_DEBUG("Maximum Vertex Count: {}", output);
		glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &output);
		BOOM_DEBUG("Maximum Indicies Count: {}", output);
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &output);
		BOOM_DEBUG("Maximum texture size: {}", output);

		GLint viewport[2];
		glGetIntegerv(GL_MAX_VIEWPORT_DIMS, viewport);
		BOOM_DEBUG("Maximum Viewport Dimensions: {} x {}", viewport[0], viewport[1]);

		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &output);
		BOOM_DEBUG("Maximum generic vertex attributes: {}", output);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &output);
		BOOM_DEBUG("Maximum vertex buffer bindings: {}", output);

		BOOM_DEBUG('\n');
	}
}