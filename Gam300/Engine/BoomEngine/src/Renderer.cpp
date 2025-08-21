#include "Core.h"
#include "Graphics/Renderer.h"

namespace {
	//helper functions
	char const* GetGlewString(GLenum name, bool isError = false) {
		if (isError)
			return reinterpret_cast<char const*>(glewGetErrorString(name));
		else
			return reinterpret_cast<char const*>(glewGetString(name));
	}
}

namespace Boom {
	GraphicsRenderer::GraphicsRenderer(int32_t w, int32_t h)
		: width{ w }
		, height{ h } 
	{
		GLenum err = glewInit();
#ifdef BOOM_ENABLE_LOG
		if (GLEW_OK != err) {
			BOOM_FATAL("Unable to initialize GLEW - error: {} abort program.\n", GetGlewString(err, true));
			std::exit(EXIT_FAILURE);
		}
		if (GLEW_VERSION_3_3) {
			BOOM_INFO("Using glew version: {}", GetGlewString(GLEW_VERSION));
			//BOOM_INFO("Driver supports OpenGL {}.{}", GetGlewString(GLEW_VERSION_MAJOR), GetGlewString(GLEW_VERSION_MINOR));
		}
		else {
			BOOM_WARN("Warning: The driver may lack full compatibility with OpenGL 3.3, potentially limiting access to advanced features.");
		}

		PrintSpecs();
#endif
	}

	BOOM_INLINE void GraphicsRenderer::OnStart() {

	}
	BOOM_INLINE void GraphicsRenderer::OnUpdate() {
		glClearColor(0.5f, 0.5f, 0.5f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		//rendering blah...
	}
	BOOM_INLINE GraphicsRenderer::~GraphicsRenderer() {

	}

	void GraphicsRenderer::PrintSpecs() {
		//std::cout << reinterpret_cast<char const*>(glewGetString(GL_VENDOR)) << std::endl;
		//BOOM_DEBUG("GPU Vendor: {}\n", GetGlewString(GL_VENDOR));
		//BOOM_DEBUG("GPU Renderer: {}", GetGlewString(GL_RENDERER));
		//BOOM_DEBUG("GPU Version: {}", GetGlewString(GL_VERSION));
		//BOOM_DEBUG("GPU Shader Version: {}", GetGlewString(GL_SHADING_LANGUAGE_VERSION));

		GLint ver[2];
		glGetIntegerv(GL_MAJOR_VERSION, &ver[0]);
		glGetIntegerv(GL_MINOR_VERSION, &ver[1]);
		BOOM_INFO("GL Version: {}.{}", ver[0], ver[1]);

		GLboolean isDB;
		glGetBooleanv(GL_DOUBLEBUFFER, &isDB);
		if (isDB)
			BOOM_INFO("Current OpenGL Context is double-buffered");
		else
			BOOM_INFO("Current OpenGL Context is not double-buffered");

		GLint output;
		glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &output);
		BOOM_INFO("Maximum Vertex Count: {}", output);
		glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &output);
		BOOM_INFO("Maximum Indicies Count: {}", output);
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &output);
		BOOM_INFO("Maximum texture size: {}", output);

		GLint viewport[2];
		glGetIntegerv(GL_MAX_VIEWPORT_DIMS, viewport);
		BOOM_INFO("Maximum Viewport Dimensions: {} x {}", viewport[0], viewport[1]);

		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &output);
		BOOM_INFO("Maximum generic vertex attributes: {}", output);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &output);
		BOOM_INFO("Maximum vertex buffer bindings: {}", output);
	}
}