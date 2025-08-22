#pragma once
#include "Core.h"

namespace Boom {
	class FrameBuffer {
	public:
		BOOM_INLINE FrameBuffer(int32_t w, int32_t h)
			: buffId{}, render{}, color{},
			width{ w }, height{ h }
		{
			//glSampleCoverage(1.f, GL_FALSE); //for multisampling
			glGenFramebuffers(1, &buffId);
			glBindFramebuffer(GL_FRAMEBUFFER, buffId);

			CreateColorAttachment();
			CreateRenderBuffer();

			uint32_t attachments[1] {
				GL_COLOR_ATTACHMENT0
			};

			glDrawBuffers(1, attachments);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				BOOM_ERROR("FrameBuffer() - frame buffer status failed.");
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		BOOM_INLINE ~FrameBuffer() {
			glDeleteTextures(1, &color);
			glDeleteRenderbuffers(1, &render);
			glDeleteFramebuffers(1, &buffId);
		}

		[[nodiscard]] BOOM_INLINE float Ratio() const {
			return (float)width / (float)height;
		}
		BOOM_INLINE void Resize(int32_t w, int32_t h) {
			width = w;
			height = h;

			//resize color attachment
			glBindTexture(GL_TEXTURE_2D, color);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);

			//resize depth attachment
			glBindRenderbuffer(GL_RENDERBUFFER, render);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}
		[[nodiscard]] BOOM_INLINE uint32_t GetTexture() const {
			return color;
		}
		
		BOOM_INLINE void Begin() {
			glBindFramebuffer(GL_FRAMEBUFFER, buffId);
			glViewport(0, 0, width, height);
			glClearColor(0.f, 0.f, 0.f, 1.f);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_SAMPLES);
			//glEnable(GL_MULTISAMPLE);
		}
		BOOM_INLINE void End() {
			//glDisable(GL_MULTISAMPLE);
			glDisable(GL_SAMPLES);
			glDisable(GL_DEPTH_TEST);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

	private:
		BOOM_INLINE void CreateColorAttachment() {
			glGenTextures(1, &color);
			glBindTexture(GL_TEXTURE_2D, color);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
		}
		BOOM_INLINE void CreateRenderBuffer() {
			glGenRenderbuffers(1, &render);
			glBindRenderbuffer(GL_RENDERBUFFER, render);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render);
		}

	private:
		uint32_t buffId;
		uint32_t render;
		uint32_t color;
		int32_t width;
		int32_t height;
	};
}