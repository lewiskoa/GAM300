#pragma once
#include "Shader.h"
#include "../Utilities/Quad.h"
#include "GlobalConstants.h"

namespace Boom {
	struct ColorShader : Shader {
		BOOM_INLINE ColorShader(std::string const& filename, glm::vec4 col)
			: Shader{ filename }
			, color{col}
			, colLoc{ GetUniformVar("color") }
			, quad{ CreateTestQuad2D() }
		{
		}
		BOOM_INLINE void Show() {
			Use();
			SetUniform(colLoc, color);
			quad->Draw();
			UnUse();
		}

	private:
		glm::vec4 color;
		int32_t colLoc;
		Quad2D quad;
	};
}