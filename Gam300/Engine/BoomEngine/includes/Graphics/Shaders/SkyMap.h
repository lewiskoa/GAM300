#pragma once
#include "Shader.h"
#include "../Utilities/Skybox.h"
#include "../Textures/Texture.h"

namespace Boom {
	struct SkyMapShader : Shader {
		BOOM_INLINE SkyMapShader(std::string const& path)
			: Shader{ path }
			, projLoc{ GetUniformVar("proj") }
			, viewLoc{ GetUniformVar("view") }
			, mapLoc{ GetUniformVar("map") }
		{
		}

		BOOM_INLINE uint32_t Generate(Texture const& tex, SkyboxMesh const& mesh, int32_t size) {
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

			//set texture source
			tex->Use(mapLoc, 0);

			uint32_t cubeMap{};
			glGenTextures(1, &cubeMap);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

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
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			//frame and render buffers
			uint32_t FBO{};
			uint32_t RBO{};
			glGenFramebuffers(1, &FBO);
			glGenFramebuffers(1, &RBO);
			glBindFramebuffer(GL_FRAMEBUFFER, FBO);
			glBindRenderbuffer(GL_RENDERBUFFER, RBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

			glViewport(0, 0, size, size);

			for (size_t i{}; i < views.size(); ++i) {
				SetUniform(viewLoc, views[i]);

				glFramebufferTexture2D(
					GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i,
					cubeMap,
					0
				);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				RenderSkyboxMesh(mesh);
			}

			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			UnUse();

			glDeleteRenderbuffers(1, &RBO);
			glDeleteFramebuffers(1, &FBO);
			return cubeMap;
		}

	private:
		int32_t projLoc;
		int32_t viewLoc;
		int32_t mapLoc;
	};
}