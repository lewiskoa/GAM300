#pragma once
#include "Shader.h"
#include "../Utilities/Data.h"
#include "../Models/Model.h"

namespace Boom {
	struct PBRShader : Shader {
		BOOM_INLINE PBRShader(std::string const& filename)
			: Shader{ filename }
			, albedoLoc{ GetUniformVar("material.albedo") }
			, roughLoc{ GetUniformVar("material.roughness") }
			, metalLoc{ GetUniformVar("material.metallic") }
			, modelMat{ GetUniformVar("modelMat") }
			, view{ GetUniformVar("view") }
			, proj{ GetUniformVar("proj") }
		{
		}
		BOOM_INLINE void SetCamera(Camera3D const& cam, Transform3D const& transform, float ratio) {
			SetUniform(proj, cam.Projection(ratio));
			SetUniform(view, cam.View(transform));
		}
		BOOM_INLINE void Draw(Mesh3D const& mesh, Transform3D const& transform) {
			SetUniform(modelMat, transform.Matrix());
			mesh->Draw();
		}
		BOOM_INLINE void Draw(Model3D const& model, Transform3D const& transform, PbrMaterial const& material) {
			SetUniform(modelMat, transform.Matrix());
			SetUniform(albedoLoc, material.albedo);
			SetUniform(roughLoc, material.roughness);
			SetUniform(metalLoc, material.metallic);
			model->Draw();
		}
	public: //testers
		BOOM_INLINE void Show(Model3D const& model) {
			Use();
			SetCamera({}, {}, 2.f);
			SetUniform(modelMat, Transform3D().Matrix());
			PbrMaterial mat{};
			SetUniform(albedoLoc, mat.albedo);
			SetUniform(roughLoc, mat.roughness);
			SetUniform(metalLoc, mat.metallic);
			model->Draw();
			UnUse();
		}
		BOOM_INLINE void Show(Mesh3D const& mesh) {
			Use();
			SetCamera({}, {}, 2.f);
			SetUniform(modelMat, Transform3D().Matrix());
			mesh->Draw();
			UnUse();
		}

	private:
		int32_t albedoLoc;
		int32_t roughLoc;
		int32_t metalLoc;

		int32_t modelMat;
		int32_t view;
		int32_t proj;
	};
}