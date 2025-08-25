#pragma once
#include "Core.h"
#include "Graphics/Buffers/Frame.h"
#include "Shaders/PBR.h"
#include "Shaders/Final.h"
#include "GlobalConstants.h"

namespace Boom {
	struct GraphicsRenderer {
	public:
		GraphicsRenderer() = delete;

		BOOM_INLINE GraphicsRenderer(int32_t w, int32_t h) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			//glewExperimental = GL_TRUE;
			GLenum err = glewInit();
#ifdef BOOM_ENABLE_LOG
			if (GLEW_OK != err) {
				BOOM_FATAL("Unable to initialize GLEW - error: {} abort program.\n", GetGlewString(err, true));
				std::exit(EXIT_FAILURE);
			}
			if (GLEW_VERSION_4_5) {
				BOOM_INFO("Using glew version: {}", GetGlewString(GLEW_VERSION));
			}
			else {
				BOOM_WARN("Warning: The driver may lack full compatibility with OpenGL 4.5, potentially limiting access to advanced features.");
			}
			PrintSpecs();
#else
			(void)err;
#endif
			finalShader = std::make_unique<FinalShader>(std::string(CONSTANTS::SHADERS_LOCATION) + "final.glsl");
			pbrShader = std::make_unique<PBRShader>(std::string(CONSTANTS::SHADERS_LOCATION) + "pbr.glsl");
			frame = std::make_unique<FrameBuffer>(w, h);
		}
		BOOM_INLINE ~GraphicsRenderer() {}

	public:
		BOOM_INLINE void SetCamera(Camera3D& cam, Transform3D const& transform) {
			pbrShader->SetCamera(cam, transform, frame->Ratio());
		}
		BOOM_INLINE void Draw(Mesh3D const& mesh, Transform3D const& transform) {
			pbrShader->Draw(mesh, transform);
		}
		BOOM_INLINE void Resize(int32_t w, int32_t h) {
			frame->Resize(w, h);
		}

		BOOM_INLINE uint32_t GetFrame() {
			return frame->GetTexture();
		}
		BOOM_INLINE void NewFrame() {
			frame->Begin();
			pbrShader->Use();
		}
		BOOM_INLINE void EndFrame() {
			pbrShader->UnUse();
			frame->End();
		}
		BOOM_INLINE void ShowFrame() {
			finalShader->Show(frame->GetTexture());
		}
	private:
		BOOM_INLINE void PrintSpecs() {
			//?? missing enum?
			//BOOM_INFO("GPU Vendor: {}", GetGlewString(GL_VENDOR));
			//BOOM_INFO("GPU Renderer: {}", GetGlewString(GL_RENDERER));
			//BOOM_INFO("GPU Version: {}", GetGlewString(GL_VERSION));
			//BOOM_INFO("GPU Shader Version: {}", GetGlewString(GL_SHADING_LANGUAGE_VERSION));

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
			BOOM_INFO("Maximum vertex buffer bindings: {}\n", output);
		}
		std::string GetGlewString(GLenum name, bool isError = false) {
			if (isError)
				return reinterpret_cast<char const*>(glewGetErrorString(name));
			else {
				char const* ret{ reinterpret_cast<char const*>(glewGetString(name)) };
				return ret ? ret : "Unknown glewGetString(" + std::to_string(name) + ')';
			}
		}
	private:
		std::unique_ptr<FrameBuffer> frame;
		std::unique_ptr<FinalShader> finalShader;
		std::unique_ptr<PBRShader> pbrShader;
	};
}