#pragma once
#include "Shader.h"
#include "../Utilities/Skybox.h"

namespace Boom
{
    struct PrefilteredShader : Shader
    {
        BOOM_INLINE PrefilteredShader(const std::string& path) : Shader(path)
        {
            u_Roughness = glGetUniformLocation(shaderId, "u_roughness");
            u_CubeMap = glGetUniformLocation(shaderId, "u_cubemap");
            u_View = glGetUniformLocation(shaderId, "u_view");
            u_Proj = glGetUniformLocation(shaderId, "u_proj");
        }

        BOOM_INLINE uint32_t Generate(uint32_t skyCubMap, SkyboxMesh& mesh, int32_t size)
        {
            // Save state
            GLint oldVP[4];
            glGetIntegerv(GL_VIEWPORT, oldVP);
            GLint oldDrawFBO = 0, oldReadFBO = 0;
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldDrawFBO);
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldReadFBO);

            // Views (unchanged)
            glm::mat4 views[] =
            {
                glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
            };
            glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

            // Create prefiltered cube
            uint32_t prefilteredMap = 0u;
            glGenTextures(1, &prefilteredMap);
            glBindTexture(GL_TEXTURE_CUBE_MAP, prefilteredMap);

            // --- IMPORTANT: allocate ALL mip levels up-front for EACH face ---
            // Number of mip levels you intend to render
            const uint32_t nbrMipLevels = 5;

            // Option A (manual per level, per face allocation - works everywhere):
            for (uint32_t mip = 0; mip < nbrMipLevels; ++mip)
            {
                const int w = std::max(1, (int)(size * std::pow(0.5, mip)));
                const int h = std::max(1, (int)(size * std::pow(0.5, mip)));
                for (uint32_t face = 0; face < 6; ++face)
                {
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip,
                        GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, nullptr);
                }
            }

            // (If you have GL 4.2+, you could instead do:
            // glTexStorage2D(GL_TEXTURE_CUBE_MAP, nbrMipLevels, GL_RGB16F, size, size);
            // which allocates all faces & mips in one call.)

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // <-- mipmapped
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // (Optional) Clamp visible mips to [0, nbrMipLevels-1]
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, (GLint)nbrMipLevels - 1);

            // Shader setup
            glUseProgram(shaderId);
            glUniformMatrix4fv(u_Proj, 1, GL_FALSE, glm::value_ptr(projection));

            // Source env (sky cubemap)
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubMap);
            glUniform1i(u_CubeMap, 0);

            // FBO + depth RBO
            uint32_t FBO = 0, RBO = 0;
            glGenFramebuffers(1, &FBO);
            glGenRenderbuffers(1, &RBO);
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glBindRenderbuffer(GL_RENDERBUFFER, RBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);

            // Render each mip
            for (uint32_t mip = 0; mip < nbrMipLevels; ++mip)
            {
                const int mipWidth = std::max(1, (int)(size * std::pow(0.5, mip)));
                const int mipHeight = std::max(1, (int)(size * std::pow(0.5, mip)));

                glBindRenderbuffer(GL_RENDERBUFFER, RBO);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
                glViewport(0, 0, mipWidth, mipHeight);

                const float roughness = (float)mip / (float)(nbrMipLevels - 1);
                glUniform1f(u_Roughness, roughness);

                for (uint32_t face = 0; face < 6; ++face)
                {
                    glUniformMatrix4fv(u_View, 1, GL_FALSE, glm::value_ptr(views[face]));
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                        GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, prefilteredMap, mip);

                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    RenderSkyboxMesh(mesh);
                }
            }

            // Restore FBO + viewport + bindings
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldDrawFBO);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, oldReadFBO);
            glViewport(oldVP[0], oldVP[1], oldVP[2], oldVP[3]);

            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glUseProgram(0);

            glDeleteRenderbuffers(1, &RBO);
            glDeleteFramebuffers(1, &FBO);
            return prefilteredMap;
        }


    private:
        uint32_t u_Roughness = 0u;
        uint32_t u_CubeMap = 0u;
        uint32_t u_View = 0u;
        uint32_t u_Proj = 0u;
    };
}