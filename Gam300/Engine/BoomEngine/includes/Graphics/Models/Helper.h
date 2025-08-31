#pragma once
#include "../Buffers/Mesh.h"
#include <assimp/quaternion.h>
#include <assimp/matrix4x4.h>
#include <assimp/color4.h>

namespace Boom {
	BOOM_INLINE static glm::vec3 AssimpToVec3(aiVector3D const& vec) {
		return { vec.x, vec.y, vec.z };
	}

	BOOM_INLINE static glm::vec4 AssmipToVec4(aiColor4D const& col) {
		return { col.r, col.g, col.b, col.a };
	}

	BOOM_INLINE static glm::quat AssimpToQuat(aiQuaternion const& quat) {
		return { quat.w, quat.x, quat.y, quat.z };
	}

	//assimp showcases like a chessboard (row major layout)
		// Row: letters(a,b,c,...)
		// Col: numbers(1,2,3,...)
		// needs to transpose manually
	BOOM_INLINE static glm::mat4 AssimpToMat4(aiMatrix4x4 const& mat) {
		return {
			mat.a1, mat.b1, mat.c1, mat.d1, //col 1
			mat.a2, mat.b2, mat.c2, mat.d2, //col 2
			mat.a3, mat.b3, mat.c3, mat.d3, //col 3
			mat.a4, mat.b4, mat.c4, mat.d4  //col 4
		};
	}
}