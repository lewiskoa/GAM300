#pragma once
#include "../Buffers/Mesh.h"
#include "../Textures/Texture.h"
#include "BoomProperties.h"


//helper functions
namespace Boom {
	BOOM_INLINE glm::highp_mat4 GetRotationMatrix(glm::vec3 const& rot) {
		return glm::toMat4(glm::quat(glm::radians(rot)));
	}
}

namespace Boom {
	

	struct Transform3D {
		BOOM_INLINE Transform3D() : translate{}, rotate{}, scale{ 1.f } {}
		BOOM_INLINE Transform3D(Transform3D const& t) = default;
		BOOM_INLINE Transform3D(glm::vec3 t, glm::vec3 r, glm::vec3 s)
			: translate{ t }
			, rotate{ r }
			, scale{ s }
		{
		}

		BOOM_INLINE glm::mat4 Matrix() const {
			return //TRS
				glm::translate(glm::mat4(1.f), translate) *
				GetRotationMatrix(rotate) *
				glm::scale(glm::mat4(1.f), scale);
		}

		glm::vec3 translate;
		glm::vec3 rotate;
		glm::vec3 scale;

		XPROPERTY_DEF
		("Transform3D", Transform3D
			, obj_member<"translate", &Transform3D::translate>
			, obj_member<"rotate", &Transform3D::rotate>
			, obj_member<"scale", &Transform3D::scale>
		)

	};

	struct Camera3D {
		//transform here refers to the camera's transformation variables
		BOOM_INLINE glm::mat4 Frustum(Transform3D const& transform, float ratio) const {
			return Projection(ratio) * View(transform);
		}
		//transform here refers to the camera's transformation variables
		BOOM_INLINE glm::mat4 View(Transform3D const& transform) const {
			glm::quat rotQuat{ glm::radians(transform.rotate) };
			glm::vec3 forward{ rotQuat * glm::vec3{0.f, 0.f, -1.f} };
			glm::vec3 up{ rotQuat * glm::vec3{0.f, 1.f, 0.f} };
			
			return glm::lookAt(
				transform.translate,			//position
				transform.translate + forward, 	//target
				up								//upwards direction
				);
		}
		BOOM_INLINE glm::mat4 Projection(float ratio) const {
			return glm::perspective(glm::radians(FOV), ratio, nearPlane, farPlane);
		}

		float nearPlane{0.3000f};
		float farPlane{1000.f};
		float FOV{45.f};

		XPROPERTY_DEF(
			"Camera3D", Camera3D,
			obj_member<"nearPlane", &Camera3D::nearPlane>,
			obj_member<"farPlane", &Camera3D::farPlane>,
			obj_member<"FOV", &Camera3D::FOV>
		)
	};

	struct PbrMaterial {
		BOOM_INLINE PbrMaterial()
			: emissive{}
			, albedo{ glm::vec3(1.f) }
			, roughness{ 0.4f }
			, metallic{ 0.5f }
			, occlusion{ 1.f }
			, occlusionMap{}, roughnessMap{}, metallicMap{}, emissiveMap{}, albedoMap{}, normalMap{}
		{
		}
		BOOM_INLINE PbrMaterial(PbrMaterial const&) = default;
		BOOM_INLINE PbrMaterial(glm::vec3 em, glm::vec3 alb, float rough, float metal, float occlu)
			: emissive{ em }
			, albedo{ alb }
			, roughness{ rough }
			, metallic{ metal }
			, occlusion{ occlu }
			, occlusionMap{}, roughnessMap{}, metallicMap{}, emissiveMap{}, albedoMap{}, normalMap{}
		{
		}
		glm::vec3 emissive;
		glm::vec3 albedo;
		float roughness;
		float metallic;
		float occlusion;

		Texture occlusionMap;
		Texture roughnessMap;
		Texture metallicMap;
		Texture emissiveMap;
		Texture albedoMap;
		Texture normalMap;

		// ===== PbrMaterial =====
		XPROPERTY_DEF(
			"PbrMaterial", PbrMaterial,
			obj_member<"emissive", &PbrMaterial::emissive>,
			obj_member<"albedo", &PbrMaterial::albedo>,
			obj_member<"roughness", &PbrMaterial::roughness>,
			obj_member<"metallic", &PbrMaterial::metallic>,
			obj_member<"occlusion", &PbrMaterial::occlusion>,
			obj_member<"occlusionMap", &PbrMaterial::occlusionMap>,
			obj_member<"roughnessMap", &PbrMaterial::roughnessMap>,
			obj_member<"metallicMap", &PbrMaterial::metallicMap>,
			obj_member<"emissiveMap", &PbrMaterial::emissiveMap>,
			obj_member<"albedoMap", &PbrMaterial::albedoMap>,
			obj_member<"normalMap", &PbrMaterial::normalMap>
		)


	};

	struct PointLight {
		BOOM_INLINE PointLight(glm::vec3 radi = glm::vec3(1.f), float intense = 1.f)
			: radiance{ radi }, intensity{ intense } 
		{
		
		}

		glm::vec3 radiance;
		float intensity;

		// ===== PointLight =====
		XPROPERTY_DEF(
			"PointLight", PointLight,
			obj_member<"radiance", &PointLight::radiance>,
			obj_member<"intensity", &PointLight::intensity>
		)

	};

	struct DirectionalLight {
		BOOM_INLINE DirectionalLight(glm::vec3 radi = glm::vec3(1.f), float intense = 2.f)
			: radiance{ radi }, intensity{ intense } 
		{
		}

		glm::vec3 radiance;
		float intensity;

		// ===== DirectionalLight =====
		XPROPERTY_DEF(
			"DirectionalLight", DirectionalLight,
			obj_member<"radiance", &DirectionalLight::radiance>,
			obj_member<"intensity", &DirectionalLight::intensity>
		)

	};

	struct SpotLight {
		//falloff and cutoff angles should be inputted as degrees
		BOOM_INLINE SpotLight(
			glm::vec3 radi = glm::vec3(1.f),
			float intense = 3.5f,
			float fall = 60.5f,
			float cut = 20.f
		)
			: radiance{ radi }
			, intensity{ intense }
			, fallOff{ glm::radians(fall) }
			, cutOff{ glm::radians(cut) }
		{
			assert(fallOff > cutOff && "Innter cone angle must be smaller than outer cone");
		}

		glm::vec3 radiance;
		float intensity;
		float fallOff;
		float cutOff;

		// ===== SpotLight =====
		XPROPERTY_DEF(
			"SpotLight", SpotLight,
			obj_member<"radiance", &SpotLight::radiance>,
			obj_member<"intensity", &SpotLight::intensity>,
			obj_member<"fallOff", &SpotLight::fallOff>,  // stored in radians
			obj_member<"cutOff", &SpotLight::cutOff>    // stored in radians
		)

	};

	struct Skybox {
		uint32_t cubeMap{};

		// ===== Skybox =====
		XPROPERTY_DEF(
			"Skybox", Skybox,
			obj_member<"cubeMap", &Skybox::cubeMap>
		)


	};
}