#pragma once
#include "Shader.h"
#include "../Utilities/Quad.h"

namespace Boom {
	struct FinalShader : Shader {
		BOOM_INLINE FinalShader(std::string const& filename)
			: Shader{ filename }
			, map{ (uint32_t)GetUniformVar("map") }
			, quad{ CreateQuad2D() }
		{
		}
		BOOM_INLINE void SetSceneMap(uint32_t m) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m);
			glUniform1i(m, 0);
		}
		BOOM_INLINE void Show(uint32_t m) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearColor(0.f, 0.f, 0.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);

			Use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m);
			glUniform1i(m, 0);
			quad->Draw(GL_TRIANGLES);
			UnUse();
		}

	private:
		uint32_t map;
		Quad2D quad;
	};
}