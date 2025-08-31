#pragma once
#include "Shader.h"
#include "../Utilities/Skybox.h"
#include "../Utilities/Data.h"

namespace Boom {
	struct SkyboxShader : Shader {
		BOOM_INLINE SkyboxShader(std::string const& path)
			: Shader{ path }
			, modelMatLoc{ GetUniformVar("modelMat") }
			, projLoc{ GetUniformVar("proj") }
			, viewLoc{ GetUniformVar("view") }
			, mapLoc{ GetUniformVar("map") }
		{
		}

		BOOM_INLINE void SetCamera(Camera3D const& cam, Transform3D const& transform, float ratio) {
			Use();
			SetUniform(projLoc, cam.Projection(ratio));
			SetUniform(viewLoc, cam.View(transform));
		}

		BOOM_INLINE void Draw(SkyboxMesh const& mesh, uint32_t cubeMap, Transform3D const& transform) {
			glm::mat4 model{ GetRotationMatrix(transform.rotate) };

			Use();
			SetUniform(modelMatLoc, model);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
			SetUniform(mapLoc, 0);
			RenderSkyboxMesh(mesh);
		}

	private:
		int32_t modelMatLoc;
		int32_t projLoc;
		int32_t viewLoc;
		int32_t mapLoc;
	};
}