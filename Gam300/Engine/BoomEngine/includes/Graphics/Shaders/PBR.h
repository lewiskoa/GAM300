#pragma once
#include "Shader.h"
#include "../Utilities/Data.h"

namespace Boom {
	struct PBRShader : Shader {
		BOOM_INLINE PBRShader(std::string const& filename)
			: Shader{ filename }
			, modelMat{ GetUniformVar("modelMat") }
			, view{ GetUniformVar("view") }
			, proj{ GetUniformVar("proj") }
		{
		}
		BOOM_INLINE void SetCamera(Camera3D const& cam, Transform3D const& transform, float ratio) {
			SetUniform(proj, cam.Projection(ratio));
			SetUniform(view, cam.View(transform));
		}
		BOOM_INLINE void Draw(Mesh3D const& mesh, Transform3D const& transform) {
			SetUniform(modelMat, transform.Matrix());
			mesh->Draw();
		}

	private:
		int32_t modelMat;
		int32_t view;
		int32_t proj;
	};
}