#pragma once
#include "Core.h"

namespace Boom {
	struct QuadVert {
		glm::vec2 pos{};
		glm::vec2 uv{};
	};
	struct FlatVert {
		glm::vec3 pos{};
		glm::vec4 col{};
	};
	struct ShadedVert {
		glm::vec3 pos{};
		glm::vec3 norm{};
		glm::vec2 uv{};

		glm::vec3 tangent{};
		glm::vec3 biTangent{};
	};
	struct SkyboxVert {
		glm::vec3 pos{};
	};

	template <class Vertex>
	struct MeshData {
		std::vector<uint32_t> idx;
		std::vector<Vertex> vtx;
		uint32_t drawMode{};
	};

	struct SkeletalVertex //hehe sorry darius - Amos
	{
		glm::vec3 pos = glm::vec3(0.0f);
		glm::vec3 norm = glm::vec3(0.0f);
		glm::vec2 uv = glm::vec2(0.0f);

		// for lighting
		glm::vec3 tangent = glm::vec3(0.0f);
		glm::vec3 biTangent = glm::vec3(0.0f);

		// for animation
		glm::ivec4 joints = glm::ivec4(-1);
		glm::vec4 weights = glm::vec4(0.0f);
	};
}