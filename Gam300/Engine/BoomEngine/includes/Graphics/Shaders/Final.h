#pragma once
#include "Shader.h"
#include "../Utilities/Quad.h"
#include "GlobalConstants.h"

namespace Boom {
	struct FinalShader : Shader {
		BOOM_INLINE FinalShader(std::string const& filename, glm::vec4 col = glm::vec4{1.f})
			: Shader{ filename }
			, map{ GetUniformVar("map") }
			, quad{ CreateQuad2D() }
			, colLoc{ GetUniformVar("color") }
			, color{col}
		{
		}
		BOOM_INLINE void SetSceneMap(uint32_t m) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m);
			SetUniform(map, 0);
		}
		BOOM_INLINE void Show(uint32_t m) {
			Use();
			SetSceneMap(m);
			SetUniform(colLoc, color);
			quad->Draw();
			UnUse();
		}

	private:
		int32_t map;
		Quad2D quad;

		int32_t colLoc;
		glm::vec4 color;
	};
}