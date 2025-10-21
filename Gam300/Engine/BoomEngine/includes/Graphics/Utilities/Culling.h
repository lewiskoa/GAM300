#pragma once
#include "../Utilities/Data.h"

//frustum culling helper functions
namespace Boom {
	struct FrustumPlanes { glm::vec4 planes[6]; };
	BOOM_INLINE glm::vec4 normalizePlane(const glm::vec4& plane) {
		float length = glm::length(glm::vec3(plane));
		return plane / length;
	}
	BOOM_INLINE FrustumPlanes extractFrustum(const glm::mat4& viewProjection) {
		FrustumPlanes frustumPlanes;
		//left
		frustumPlanes.planes[0] = normalizePlane(viewProjection[3] + viewProjection[0]);
		//right
		frustumPlanes.planes[1] = normalizePlane(viewProjection[3] - viewProjection[0]);
		//bottom
		frustumPlanes.planes[2] = normalizePlane(viewProjection[3] + viewProjection[1]);
		//top
		frustumPlanes.planes[3] = normalizePlane(viewProjection[3] - viewProjection[1]);
		//near
		frustumPlanes.planes[4] = normalizePlane(viewProjection[3] + viewProjection[2]);
		//far
		frustumPlanes.planes[5] = normalizePlane(viewProjection[3] - viewProjection[2]);
		return frustumPlanes;
	}
	BOOM_INLINE bool sphereInside(const FrustumPlanes& frustum, const glm::vec3& center, float radius) {
		for (int i = 0; i < 6; ++i) {
			const glm::vec4& plane = frustum.planes[i];
			if (glm::dot(glm::vec3(plane), center) + plane.w + radius < 0) {
				return false; // Sphere is outside this plane
			}
		}
		return true; // Sphere is inside all planes
	}
	BOOM_INLINE void ToWorldSphere(const Transform3D& t,
		const glm::vec3& localC, float localR,
		glm::vec3& worldC, float& worldR)
	{
		// rotate then translate the (scaled) local center
		glm::quat q = glm::quat(glm::radians(t.rotate));
		worldC = t.translate + (q * (localC * t.scale)); // per-axis scale then rotate

		// non-uniform scale to inflate by the largest axis
		float s = std::max({ std::abs(t.scale.x), std::abs(t.scale.y), std::abs(t.scale.z) });
		worldR = localR * s;
	}

}
