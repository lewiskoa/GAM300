#pragma once
#include "Shader.h"

namespace Boom {
	struct PBRShader : Shader {
		BOOM_INLINE PBRShader(std::string const& filename)
			: Shader{ filename }
			, modelMat{ glGetUniformLocation(shaderId, "modelMat") }
			, view{ glGetUniformLocation(shaderId, "view") }
			, proj{ glGetUniformLocation(shaderId, "proj") }
		{
		}

	private:
		uint32_t modelMat;
		uint32_t view;
		uint32_t proj;
	};
}