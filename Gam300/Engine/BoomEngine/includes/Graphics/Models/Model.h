#pragma once
#include "Helper.h"
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include "Animator.h"

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
			//Cheeky
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
				vert.uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
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
		std::vector<std::unique_ptr<ShadedMesh>> m_Meshes;
	};

	struct SkeletalModel : Model
	{
		using JointMap = std::unordered_map<std::string, Joint>;
		BOOM_INLINE SkeletalModel() = default;
		BOOM_INLINE SkeletalModel(const std::string& path)
		{
			Load(path);
		}
		BOOM_INLINE bool HasJoint() override final { return m_JointCount; }

		BOOM_INLINE void Load(std::string path) override final
		{
			//Cheeky
			path = CONSTANTS::MODELS_LOCAITON + path;

			uint32_t flags =	aiProcess_Triangulate |
								aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
								aiProcess_OptimizeMeshes |
								aiProcess_OptimizeGraph | aiProcess_ValidateDataStructure |
								aiProcess_ImproveCacheLocality |
								aiProcess_FixInfacingNormals |
								aiProcess_SortByPType |
								aiProcess_JoinIdenticalVertices |
								aiProcess_FlipUVs | aiProcess_GenUVCoords |
								aiProcess_LimitBoneWeights;

			Assimp::Importer importer;
			const aiScene* ai_scene = importer.ReadFile(path,
				flags);
			if (!ai_scene || ai_scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !ai_scene-> mRootNode)
			{
				BOOM_ERROR("failed to load model: ’{}’", importer.GetErrorString());
				return;
			}

			m_Animator = std::make_shared<Animator>();
			m_Animator->m_GlobalTransform = glm::inverse(AssimpToMat4(ai_scene->mRootNode-> mTransformation));

			// temp joints map
			JointMap jointMap = {};

			// parse all meshes
			ParseNode(ai_scene, ai_scene->mRootNode, jointMap);

			// parse animations
			ParseAnimations(ai_scene, jointMap);

		}

		BOOM_INLINE void Draw() override final 
		{
			for (auto& mesh : meshes) 
			{
				mesh->Draw();
			}
		}

		BOOM_INLINE auto GetAnimator() { return m_Animator; }
	private:

		BOOM_INLINE void ParseNode(const aiScene* ai_scene, aiNode* ai_node, JointMap& jointMap)
		{
			for (uint32_t i = 0; i < ai_node->mNumMeshes; i++)
			{
				ParseMesh(ai_scene->mMeshes[ai_node->mMeshes[i]], jointMap);
			}

			for (uint32_t i = 0; i < ai_node->mNumChildren; i++)
			{
				ParseNode(ai_scene, ai_node->mChildren[i], jointMap);
			}
		}

		BOOM_INLINE void SetVertexJoint(SkeletalVertex& vertex, int32_t id, float weight)
		{
			for (uint32_t i = 0; i < 4; i++)
			{
				if (vertex.joints[i] < 0)
				{
					vertex.weights[i] = weight;
					vertex.joints[i] = id;
					return;
				}
			}
		}

		BOOM_INLINE void ParseHierarchy(aiNode* ai_node, Joint& joint, JointMap& jointMap)
		{
			std::string jointName(ai_node->mName.C_Str());

			if (jointMap.count(jointName))
			{
				joint = jointMap[jointName];

				for (uint32_t i = 0; i < ai_node->mNumChildren; i++)
				{
					Joint child;
					ParseHierarchy(ai_node->mChildren[i], child, jointMap);
					joint.Children.push_back(std::move(child));
				}
			}
			else
			{
				for (uint32_t i = 0; i < ai_node->mNumChildren; i++)
				{
					ParseHierarchy(ai_node->mChildren[i], joint, jointMap);
				}
			}
		}

		BOOM_INLINE void ParseAnimations(const aiScene* ai_scene, JointMap& jointMap)
		{
			// parse animation data
			for (uint32_t i = 0; i < ai_scene->mNumAnimations; i++)
			{
				auto ai_anim = ai_scene->mAnimations[i];

				Animation animation;
				animation.Name		= ai_anim->mName.C_Str();
				animation.Duration	= (float)ai_anim->mDuration;
				animation.Speed		= (float)ai_anim->mTicksPerSecond;
				m_Animator->m_Animations.push_back(animation);

				// parse animation keys
				for (uint32_t j = 0; j < ai_anim->mNumChannels; j++)
				{
					aiNodeAnim* ai_channel = ai_anim->mChannels[j];
					auto jointName(ai_channel->mNodeName.C_Str());
					if (!jointMap.count(jointName)) { continue; }

					for (uint32_t k = 0; k < ai_channel->mNumPositionKeys; k++)
					{
						KeyFrame key;
						key.Position = AssimpToVec3(ai_channel->mPositionKeys[k].mValue);
						key.Rotation = AssimpToQuat(ai_channel->mRotationKeys[k].mValue);
						key.Scale = AssimpToVec3(ai_channel->mScalingKeys[k].mValue);
						key.TimeStamp = (float)ai_channel->mPositionKeys[k].mTime;
						jointMap[jointName].Keys.push_back(key);
					}
				}
			}

			// parse jointMap hierarchy
			ParseHierarchy(ai_scene->mRootNode, m_Animator->m_Root, jointMap);

			// initialize animator
			m_Animator->m_Transforms.resize(m_JointCount);
		}

		BOOM_INLINE void ParseMesh(const aiMesh* ai_mesh, JointMap& jointMap)
		{
			// mesh data
			MeshData<SkeletalVertex> data;

			// vertices
			for (uint32_t i = 0; i < ai_mesh->mNumVertices; i++)
			{
				SkeletalVertex vertex;
				// positions
				vertex.pos = AssimpToVec3(ai_mesh->mVertices[i]);
				// normals
				vertex.norm = AssimpToVec3(ai_mesh->mNormals[i]);
				// texcoords
				vertex.uv.x = ai_mesh->mTextureCoords[0][i].x;
				vertex.uv.y = ai_mesh->mTextureCoords[0][i].y;
				// push vertex
				data.vtx.push_back(std::move(vertex));
			}

			// indices   
			for (uint32_t i = 0; i < ai_mesh->mNumFaces; i++)
			{
				for (uint32_t k = 0; k < ai_mesh->mFaces[i].mNumIndices; k++)
				{
					data.idx.push_back(ai_mesh->mFaces[i].mIndices[k]);
				}
			}

			// joints
			for (uint32_t i = 0; i < ai_mesh->mNumBones; i++)
			{
				aiBone* ai_bone = ai_mesh->mBones[i];

				// get joint name
				std::string jointName(ai_bone->mName.C_Str());

				// add joint if not found
				if (jointMap.count(jointName) == 0)
				{
					jointMap[jointName].Offset = AssimpToMat4(ai_bone->mOffsetMatrix);
					jointMap[jointName].Index = m_JointCount++;
					jointMap[jointName].Name = jointName;
				}

				// set vertex joint weights
				for (uint32_t j = 0; j < ai_bone->mNumWeights; j++)
				{
					SetVertexJoint(data.vtx[ai_bone->mWeights[j].mVertexId], jointMap[jointName].Index, ai_bone->mWeights[j].mWeight);
				}
			}

			// create new mesh instance
			meshes.push_back(std::make_unique<SkeletalMesh>(std::move(data)));

		}

	private:
		std::vector<std::unique_ptr<SkeletalMesh>> meshes;
		std::shared_ptr<Animator> m_Animator;
		uint32_t m_JointCount = 0;
	};

	// helper type definitions
	using Animator3D = std::shared_ptr<Animator>;
	using Model3D = std::shared_ptr<Model>;
}
