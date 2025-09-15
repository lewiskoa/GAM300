#pragma once
#include "Shader.h"
#include "../Utilities/Quad.h"

namespace Boom
{
    struct BrdfShader : Shader
    {
        BOOM_INLINE BrdfShader(const std::string& path) :
            Shader(path)
        {
        }

        BOOM_INLINE uint32_t Generate(int32_t size)
        {
          /*  GLint oldVP[4];
            glGetIntegerv(GL_VIEWPORT, oldVP);
            GLint oldDrawFBO = 0, oldReadFBO = 0;
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldDrawFBO);
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldReadFBO);*/
            uint32_t brdfMap = 0u;
            glGenTextures(1, &brdfMap);
            glBindTexture(GL_TEXTURE_2D, brdfMap);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, size, size, 0, GL_RG, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glUseProgram(shaderId);

            uint32_t FBO, RBO = 0;
            glGenFramebuffers(1, &FBO);
            glGenRenderbuffers(1, &RBO);

            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glBindRenderbuffer(GL_RENDERBUFFER, RBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfMap, 0);
           
            glViewport(0, 0, size, size);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            CreateQuad2D()->Draw();
            //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldDrawFBO);
            //glBindFramebuffer(GL_READ_FRAMEBUFFER, oldReadFBO);

            //// restore viewport
            //glViewport(oldVP[0], oldVP[1], oldVP[2], oldVP[3]);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glUseProgram(0);

            // delete fbo & rbo
            glDeleteRenderbuffers(1, &RBO);
            glDeleteFramebuffers(1, &FBO);
            return brdfMap;
        }
    };
}