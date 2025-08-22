#pragma once
#include "Shader.h"
#include "../Utilities/Data.h"

namespace Boom {
	struct PBRShader : Shader {
		BOOM_INLINE PBRShader(std::string const& filename)
			: Shader{ filename }
			, modelMat{ (uint32_t)GetUniformVar("modelMat") }
			, view{ (uint32_t)GetUniformVar("view") }
			, proj{ (uint32_t)GetUniformVar("proj") }
		{
		}
		BOOM_INLINE void SetCamera(Camera3D const& cam, Transform3D const& transform, float ratio) {
			SetUniform(cam.Projection(ratio));
			SetUniform(cam.View(transform));
		}
		BOOM_INLINE void Draw(Mesh3D const& mesh, Transform3D const& transform) {
			SetUniform(transform.Matrix());
			mesh->Draw(GL_TRIANGLES);
		}

	private:
		uint32_t modelMat;
		uint32_t view;
		uint32_t proj;
	};
}