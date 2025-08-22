#pragma once
#include "Vertex.h"

namespace Boom {
	template <class Vertex>
	class Mesh {
	public:
		BOOM_INLINE Mesh() = default; //to be decided what empty mesh should be.

		//creates the mesh data in storage for the provided vertex data
		BOOM_INLINE Mesh(MeshData<Vertex> const& data)
			: numVtx{ (uint32_t)data.vtx.size() }
			, numIdx{ (uint32_t)data.idx.size() }
			, buffId{} {
			if (data.vtx.empty()) {
				BOOM_ERROR("Mesh() - empty construct");
				return;
			}

			//gen + bind vtx buffer array
			glGenVertexArrays(1, &buffId);
			glBindVertexArray(buffId);

			//create vtx buff
			uint32_t VBO{};
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(
				GL_ARRAY_BUFFER,
				numVtx * sizeof(Vertex),
				data.vtx.data(),
				GL_STATIC_DRAW
			);

			//element buffer if theres indicies
			if (numIdx) {
				uint32_t EBO{};
				glGenBuffers(1, &EBO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
				glBufferData(
					GL_ELEMENT_ARRAY_BUFFER,
					numIdx * sizeof(uint32_t),
					data.idx.data(),
					GL_STATIC_DRAW
				);
			}

			//vert types
			if (TypeID<Vertex>() == TypeID<ShadedVert>()) {
				SetAttribute(0, 3, (void*)offsetof(ShadedVert, pos));
				SetAttribute(1, 3, (void*)offsetof(ShadedVert, norm));
				SetAttribute(2, 2, (void*)offsetof(ShadedVert, uv));
			}
			else if (TypeID<Vertex>() == TypeID<FlatVert>()) {
				SetAttribute(0, 3, (void*)offsetof(FlatVert, pos));
				SetAttribute(1, 4, (void*)offsetof(FlatVert, col));
			}
			else if (TypeID<Vertex>() == TypeID<QuadVert>()) {
				SetAttribute(0, 4, (void*)offsetof(QuadVert, data));
			}
			else {
				BOOM_ERROR(false && "Mesh() - invalid vertex type.");
			}

			glBindVertexArray(0);
		}

		//simple draw mesh function
		BOOM_INLINE void Draw(uint32_t mode) {
			glBindVertexArray(buffId);
			if (numIdx) {
				glDrawElements(mode, numIdx, GL_UNSIGNED_INT, 0);
			}
			else {
				glDrawArrays(mode, 0, numVtx);
			}
			glBindVertexArray(0);
		}

		//automatically frees storage when life over
		BOOM_INLINE ~Mesh() {
			glDeleteVertexArrays(1, &buffId);
		}
	private:
		BOOM_INLINE void SetAttribute(uint32_t index, int32_t size, void const* val) {
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, sizeof(Vertex), val);
		}

	private:
		uint32_t numVtx;
		uint32_t numIdx;
		uint32_t buffId;
	};

	//3d Mesh
	using ShadedMesh = Mesh<ShadedVert>;
	using Mesh3D = std::shared_ptr<ShadedMesh>;
}