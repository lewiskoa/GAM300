#pragma once
#include "Shader.h"
#include "../Utilities/Skybox.h"

namespace Boom {
	struct IrradianceShader : Shader {
		BOOM_INLINE IrradianceShader(std::string const& filename)
			: Shader{ filename }
			, cubeMapLoc{ GetUniformVar("cubeMap") }
			, viewLoc{ GetUniformVar("view") }
			, projLoc{ GetUniformVar("proj") }
		{
		}

		BOOM_INLINE uint32_t Generate(uint32_t skyCubeMap, SkyboxMesh const& mesh, int32_t size) {
			std::array<glm::mat4, 6> views{
				glm::lookAt(glm::vec3{}, { 1.f,  0.f,  0.f}, {0.f, -1.f,  0.f}),
				glm::lookAt(glm::vec3{}, {-1.f,  0.f,  0.f}, {0.f, -1.f,  0.f}),
				glm::lookAt(glm::vec3{}, { 0.f,  1.f,  0.f}, {0.f,  0.f,  1.f}),

				glm::lookAt(glm::vec3{}, { 0.f, -1.f,  0.f}, {0.f,  0.f, -1.f}),
				glm::lookAt(glm::vec3{}, { 0.f,  0.f,  1.f}, {0.f, -1.f,  0.f}),
				glm::lookAt(glm::vec3{}, { 0.f,  0.f, -1.f}, {0.f, -1.f,  0.f}),
			};

			glm::mat4 proj{ glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f) };

			Use();
			SetUniform(projLoc, proj);

			//gen cube map
			uint32_t irradMap{};
			glGenTextures(1, &irradMap);
			glBindTexture(GL_TEXTURE_CUBE_MAP, irradMap);

			//faces
			for (size_t i{}; i < views.size(); ++i) {
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i,
					0,
					GL_RGB16F,  //format
					size,		//width
					size,		//height
					0,
					GL_RGB,		//format
					GL_FLOAT,   //type
					NULL
				);
			}

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


			//binding skybox texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubeMap);
			SetUniform(cubeMapLoc, 0);

			uint32_t FBO{};
			uint32_t RBO{};
			glGenFramebuffers(1, &FBO);
			glGenRenderbuffers(1, &RBO);
			glBindFramebuffer(GL_FRAMEBUFFER, FBO);
			glBindRenderbuffer(GL_RENDERBUFFER, RBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);

			glViewport(0, 0, size, size);
			glBindFramebuffer(GL_FRAMEBUFFER, FBO);

			for (size_t i{}; i < views.size(); ++i) {
				SetUniform(viewLoc, views[i]);

				glFramebufferTexture2D(
					GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i,
					irradMap,
					0
				);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				RenderSkyboxMesh(mesh);
			}

			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			UnUse();

			glDeleteRenderbuffers(1, &RBO);
			glDeleteFramebuffers(1, &FBO);
			return irradMap;
		}

	private:
		int32_t cubeMapLoc;
		int32_t viewLoc;
		int32_t projLoc;
	};
}