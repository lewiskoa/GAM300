#pragma once
#include "Core.h"
#include "Graphics/Buffers/Frame.h"
#include "Shaders/PBR.h"
#include "Shaders/Final.h"

namespace Boom {
	struct GraphicsRenderer {
	public:
		GraphicsRenderer() = delete;

		BOOM_INLINE GraphicsRenderer(int32_t w, int32_t h);
		BOOM_INLINE ~GraphicsRenderer();

	public:
		BOOM_INLINE void SetCamera(Camera3D& cam, Transform3D const& transform);
		BOOM_INLINE void Draw(Mesh3D const& mesh, Transform3D const& transform);
		BOOM_INLINE void Resize(int32_t w, int32_t h);

		BOOM_INLINE uint32_t GetFrame();
		BOOM_INLINE void NewFrame();
		BOOM_INLINE void EndFrame();
		BOOM_INLINE void ShowFrame();
	private:
		BOOM_INLINE void PrintSpecs();
	private:
		std::unique_ptr<FrameBuffer> frame;
		std::unique_ptr<FinalShader> finalShader;
		std::unique_ptr<PBRShader> pbrShader;
	};
}