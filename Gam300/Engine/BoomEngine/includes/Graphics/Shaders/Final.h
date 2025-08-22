#pragma once
#include "Shader.h"
#include "../Utilities/Quad.h"

namespace Boom {
	struct FinalShader : Shader {
		BOOM_INLINE FinalShader(std::string const& filename)
			: Shader{ filename }
			, map{ glGetUniformLocation(shaderId, "map") }
			, quad{ CreateQuad2D() }
		{
		}
		BOOM_INLINE void SetSceneMap(uint32_t map) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, map);
			glUniform1i(map, 0);
		}
		BOOM_INLINE void Show(uint32_t map) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearColor(0.f, 0.f, 0.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);

			Use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, map);
			glUniform1i(map, 0);
			quad->Draw(GL_TRIANGLES);
			UnUse();
		}

	private:
		uint32_t map;
		Quad2D quad;
	};
}