#pragma once
#include "Shader.h"
#include "../Utilities/Data.h"
#include "../Models/Model.h"

namespace Boom {
	struct PBRShader : Shader {
		BOOM_INLINE PBRShader(std::string const& filename)
			: Shader{ filename }
			, noSpotLightLoc{ GetUniformVar("noSpotLight") }
			, noDirLightLoc{ GetUniformVar("noDirLight") }
			, noPointLightLoc{ GetUniformVar("noPointLight") }
			, albedoLoc{ GetUniformVar("material.albedo") }
			, roughLoc{ GetUniformVar("material.roughness") }
			, metalLoc{ GetUniformVar("material.metallic") }
			, viewPosLoc{ GetUniformVar("viewPos") }
			, frustumMatLoc{ GetUniformVar("frustumMat") }
			, modelMatLoc{ GetUniformVar("modelMat") }
		{
		}

	public: //lights
		template<class TYPE>
		void SetLight(TYPE const& light, Transform3D const& transform, int32_t index) {
			static_assert(
				std::is_same<TYPE, PointLight>::value ||
				std::is_same<TYPE, DirectionalLight>::value ||
				std::is_same<TYPE, SpotLight>::value,
				"SetLight<TYPE>() only supports Spot, Point or Directional light.");
		}
		template <>
		BOOM_INLINE void SetLight(SpotLight const& light, Transform3D const& transform, int32_t index) {
			std::string intensity{ "spotLights[" + std::to_string(index) + "].intensity" };
			std::string direction{ "spotLights[" + std::to_string(index) + "].dir" };
			std::string radiance{ "spotLights[" + std::to_string(index) + "].radiance" };
			std::string position{ "spotLights[" + std::to_string(index) + "].position" };
			std::string fallOff{ "spotLights[" + std::to_string(index) + "].fallOff" };
			std::string cutOff{ "spotLights[" + std::to_string(index) + "].cutOff" };

			SetUniform(GetUniformVar(radiance), light.radiance);
			SetUniform(GetUniformVar(direction), transform.rotate);
			SetUniform(GetUniformVar(intensity), light.intensity);
			SetUniform(GetUniformVar(position), transform.translate);
			SetUniform(GetUniformVar(fallOff), light.fallOff);
			SetUniform(GetUniformVar(cutOff), light.cutOff);
		}
		template <>
		BOOM_INLINE void SetLight(DirectionalLight const& light, Transform3D const& transform, int32_t index) {
			std::string intensity{ "dirLights[" + std::to_string(index) + "].intensity" };
			std::string direction{ "dirLights[" + std::to_string(index) + "].dir" };
			std::string radiance{ "dirLights[" + std::to_string(index) + "].radiance" };

			SetUniform(GetUniformVar(radiance), light.radiance);
			SetUniform(GetUniformVar(direction), transform.rotate);
			SetUniform(GetUniformVar(intensity), light.intensity);
		}
		template <>
		BOOM_INLINE void SetLight(PointLight const& light, Transform3D const& transform, int32_t index) {
			std::string intensity{ "pointLights[" + std::to_string(index) + "].intensity" };
			std::string radiance{ "pointLights[" + std::to_string(index) + "].radiance" };
			std::string position{ "pointLights[" + std::to_string(index) + "].position" };

			SetUniform(GetUniformVar(position), transform.translate);
			SetUniform(GetUniformVar(radiance), light.radiance);
			SetUniform(GetUniformVar(intensity), light.intensity);
		}

		BOOM_INLINE void SetSpotLightCount(int32_t count) {
			SetUniform(noSpotLightLoc, count);
		}
		BOOM_INLINE void SetDirectionalLightCount(int32_t count) {
			SetUniform(noDirLightLoc, count);
		}
		BOOM_INLINE void SetPointLightCount(int32_t count) {
			SetUniform(noPointLightLoc, count);
		}

	public:
		BOOM_INLINE void SetCamera(Camera3D const& cam, Transform3D const& transform, float ratio) {
			SetUniform(frustumMatLoc, cam.Frustum(transform, ratio));
			SetUniform(viewPosLoc, transform.translate);
		}
		BOOM_INLINE void Draw(Mesh3D const& mesh, Transform3D const& transform) {
			SetUniform(modelMatLoc, transform.Matrix());
			mesh->Draw();
		}
		BOOM_INLINE void Draw(Model3D const& model, Transform3D const& transform, PbrMaterial const& material) {
			SetUniform(modelMatLoc, transform.Matrix());
			SetUniform(albedoLoc, material.albedo);
			SetUniform(roughLoc, material.roughness);
			SetUniform(metalLoc, material.metallic);
			model->Draw();
		}

	private:
		int32_t noSpotLightLoc;
		int32_t noDirLightLoc;
		int32_t noPointLightLoc;

		int32_t albedoLoc;
		int32_t roughLoc;
		int32_t metalLoc;

		int32_t viewPosLoc;
		int32_t frustumMatLoc;
		int32_t modelMatLoc;
	};
}