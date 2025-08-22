#pragma once
#include "../Buffers/Mesh.h"

namespace Boom {
	BOOM_INLINE glm::highp_mat4 GetRotationMatrix(glm::vec3 const& rot) {
		return glm::toMat4(glm::quat(glm::radians(rot)));
	}

	struct Transform3D {
		BOOM_INLINE glm::mat4 Matrix() const {
			return
				glm::translate(glm::mat4(1.f), translate) *
				GetRotationMatrix(rotate) *
				glm::scale(glm::mat4(1.f), scale);
		}

		glm::vec3 translate{};
		glm::vec3 rotate{};
		glm::vec3 scale{1.f};
	};

	struct Camera3D {
		BOOM_INLINE glm::mat4 Frustum(Transform3D const& transform, float ratio) const {
			return Projection(ratio) * View(transform);
		}
		BOOM_INLINE glm::mat4 View(Transform3D const& transform) const {
			return glm::lookAt(
				transform.translate,							//eye
				transform.translate - glm::vec3(0.f, 0.f, 1.f), //center
				glm::vec3(0, 1, 0)								//up
				) * GetRotationMatrix(transform.rotate);
		}
		BOOM_INLINE glm::mat4 Projection(float ratio) const {
			return glm::perspective(FOV, ratio, nearPlane, farPlane);
		}

		float nearPlane{0.3f};
		float farPlane{1000.f};
		float FOV{45.f};
	};
}