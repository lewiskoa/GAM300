#pragma once
#include "../Buffers/Mesh.h"

namespace Boom {
	using SkyboxMesh = std::unique_ptr<Mesh<SkyboxVert>>;

	BOOM_INLINE void RenderSkyboxMesh(SkyboxMesh const& mesh) {
		//setup draw options for mesh
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);

		mesh->Draw(GL_TRIANGLES);

		//reset draw options
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_CULL_FACE);
	}

	BOOM_INLINE SkyboxMesh CreateSkyboxMesh() {
		//cube vertices (anti-clockwise)
		std::vector<glm::vec3> vert {
			//front
			{-1.f, -1.f, -1.f}, //0
			{ 1.f, -1.f, -1.f}, //1
			{ 1.f,  1.f, -1.f}, //2
			{-1.f,  1.f, -1.f}, //3

			//back
			{-1.f, -1.f,  1.f}, //4
			{ 1.f, -1.f,  1.f}, //5
			{ 1.f,  1.f,  1.f}, //6
			{-1.f,  1.f,  1.f}  //7
		};
		std::vector<uint32_t> idx {
			0, 1, 2, 0, 2, 3, //front
			5, 4, 7, 5, 7, 6, //back
			4, 0, 3, 4, 3, 7, //left
			1, 5, 6, 1, 6, 2, //right
			3, 2, 6, 3, 6, 7, //top
			5, 1, 0, 5, 0, 4  //btm
		};

		MeshData<SkyboxVert> meshData;
		
		for (auto& v : vert) {
			meshData.vtx.push_back({ v });
		}
		for (auto& i : idx) {
			meshData.idx.push_back(i);
		}

		return std::make_unique<Mesh<SkyboxVert>>(std::move(meshData));
	}
}