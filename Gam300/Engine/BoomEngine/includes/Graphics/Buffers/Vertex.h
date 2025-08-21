#pragma once
#include "Core.h"

namespace Boom {
	struct QuadVert {
		float data[4]{};
	};
	struct FlatVert {
		glm::vec3 pos{};
		glm::vec4 col{};
	};
	struct ShadedVert {
		glm::vec3 pos{};
		glm::vec3 norm{};
		glm::vec2 uv{};
	};

	template <class Vertex>
	struct MeshData {
		std::vector<uint32_t> idx;
		std::vector<Vertex> vtx;
	};
}