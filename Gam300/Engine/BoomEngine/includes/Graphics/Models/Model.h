#pragma once
#include "Helper.h"
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace Boom {
	struct Model
	{
		BOOM_INLINE virtual bool HasJoint() { return false; }
		BOOM_INLINE virtual void Load(std::string) {}
		BOOM_INLINE virtual void Draw() {}
	};

	struct StaticModel : Model {
		BOOM_INLINE StaticModel() = default;

		BOOM_INLINE StaticModel(const std::string& path)
		{
			Load(path);
		}

		//initializes an assimp importer, reads model file
		//applies flags for optimization and then parses loaded scene's meshes
		void Load(std::string path)
			override final
		{

			path = CONSTANTS::MODELS_LOCAITON + path;

			uint32_t flags = aiProcess_Triangulate |
				aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
				aiProcess_OptimizeMeshes |
				aiProcess_OptimizeGraph | aiProcess_ValidateDataStructure |
				aiProcess_ImproveCacheLocality |
				aiProcess_FixInfacingNormals |
				aiProcess_GenUVCoords | aiProcess_FlipUVs;

			Assimp::Importer importer;
			const aiScene* ai_scene = importer.ReadFile(path,
				flags);

			if (!ai_scene || ai_scene->mFlags ==
				AI_SCENE_FLAGS_INCOMPLETE || !ai_scene->mRootNode)
			{
				BOOM_ERROR("failed to load model-{}",
					importer.GetErrorString());
				return;
			}

			// parse all meshes
			ParseNode(ai_scene, ai_scene->mRootNode);
		}

		BOOM_INLINE void Draw() {
			for (auto& mesh : meshes) {
				mesh->Draw();
			}
		}
	private:
		//Parses the node's meshes and all its childrens' meshes recursively
		BOOM_INLINE void ParseNode(aiScene const* scene, aiNode* node) {
			for (uint32_t i{}; i < node->mNumMeshes; ++i) {
				ParseMesh(scene->mMeshes[node->mMeshes[i]]);
			}

			for (uint32_t i{}; i < node->mNumChildren; ++i) {
				ParseNode(scene, node->mChildren[i]);
			}
		}

		BOOM_INLINE void ParseMesh(aiMesh* mesh) {
			MeshData<ShadedVert> meshData{};

			//vertex data
			for (uint32_t i{}; i < mesh->mNumVertices; ++i) {
				ShadedVert vert;
				vert.pos = AssimpToVec3(mesh->mVertices[i]);
				vert.norm = AssimpToVec3(mesh->mNormals[i]);
				vert.uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
				vert.biTangent = glm::normalize(AssimpToVec3(mesh->mBitangents[i]));
				vert.tangent = glm::normalize(AssimpToVec3(mesh->mTangents[i]));

				meshData.vtx.push_back(vert);
			}

			//indicies data
			for (uint32_t i{}; i < mesh->mNumFaces; ++i) {
				for (uint32_t j{}; j < mesh->mFaces[i].mNumIndices; ++j) {
					meshData.idx.push_back(mesh->mFaces[i].mIndices[j]);
				}
			}

			meshData.drawMode = GL_TRIANGLES; //default draw mode

			auto ret{ std::make_unique<ShadedMesh>(std::move(meshData)) };
			meshes.push_back(std::move(ret));
		}
	private:
		std::vector<Mesh3D> meshes;
	};

	using Model3D = std::shared_ptr<Model>;
}