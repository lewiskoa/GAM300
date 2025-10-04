#include "Core.h"
#include "Graphics/Textures/Texture.h"
#include "GlobalConstants.h"

#pragma warning(push)
#pragma warning(disable : 4244 4267 4458 4100 5054 4189) //library warnings ignored
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <gli/gli.hpp>
#pragma warning(pop)

namespace Boom {
	Texture2D::Texture2D(std::string filename, bool isFlipY, bool isHDR)
		: height{}, width{}, id{}
	{
		filename = CONSTANTS::TEXTURES_LOCATION.data() + filename;

		std::string ext{ GetExtension(filename) };
		if (ext == "dds") {
			LoadCompressed(filename, isFlipY);
		}
		else {
			LoadUnCompressed(filename, isFlipY, isHDR);
		}
	}
	Texture2D::~Texture2D() {
		glDeleteTextures(1, &id);
	}

	void Texture2D::LoadUnCompressed(std::string const& filename, bool isFlipY, bool isHDR) {
		//flip y axis (needed operation for many image types)
		stbi_set_flip_vertically_on_load(isFlipY);

		//texture data
		void* pixels{};

		if (isHDR) {
			int32_t channels{};
			pixels = stbi_loadf(filename.c_str(), &width, &height, &channels, 0);
		}
		else {
			pixels = stbi_load(filename.c_str(), &width, &height, nullptr, 4);
		}
		if (pixels == nullptr) {
			BOOM_ERROR("failed Texture2D::Load({})", filename);
			return;
		}

		//texture buffers
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		if (isHDR) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, (float*)pixels);
		}
		else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (uint32_t*)pixels);
		}
		stbi_image_free(pixels);

		//options and optimization
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void Texture2D::LoadCompressed(std::string const& filename, bool isFlipY) {
		// Load DDS using GLI
		gli::texture texture = gli::load(filename);
		if (texture.empty()) {
			BOOM_ERROR("failed Texture2D::Load({}) - Could not load DDS file", filename);
			return;
		}

		// Ensure it's a 2D texture and DXT1 format
		if (texture.target() != gli::TARGET_2D) {
			BOOM_ERROR("failed Texture2D::Load({}) - Only 2D DDS textures supported", filename);
			return;
		}
		gli::gl gl(gli::gl::PROFILE_GL33); // Adjust for your OpenGL version
		gli::gl::format format = gl.translate(texture.format(), texture.swizzles());
		if (format.Internal != gli::gl::INTERNAL_RGBA_DXT1) {
			BOOM_ERROR("failed Texture2D::Load({}) - DDS must be DXT1 format", filename);
			return;
		}

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		gli::texture2d tex2D(texture);
		if (isFlipY) {
			tex2D = gli::flip(tex2D);
		}
		width = (int32_t)tex2D.extent().x;
		height = (int32_t)tex2D.extent().y;
		GLsizei compressed_size = (GLsizei)tex2D.size(0);
		const void* compressed_data = tex2D.data(0, 0, 0);

		glCompressedTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
			width,
			height,
			0,
			compressed_size,
			compressed_data
		);

		if (glGetError() != GL_NO_ERROR) {
			BOOM_ERROR("failed Texture2D::Load({}) - OpenGL error during DDS upload", filename);
			glDeleteTextures(1, &id);
			return;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	//set's texture's active unit and uniform to graphics
	void Texture2D::Use(int32_t uniform, int32_t unit) {
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, id);
		glUniform1i(uniform, unit);
	}

	void Texture2D::Bind() {
		glBindTexture(GL_TEXTURE_2D, id);
	}
	void Texture2D::UnBind() {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	Texture2D::operator uint32_t() const noexcept { return id; }
	int32_t Texture2D::Height() const noexcept { return height; }
	int32_t Texture2D::Width() const noexcept { return width; }

	std::string Texture2D::GetExtension(std::string const& filename) {
		uint32_t pos{ (uint32_t)filename.find_last_of('.') };
		if (pos == std::string::npos || pos == filename.length() - 1) {
			return ""; //no extension
		}
		std::string ext{ filename.substr(pos + 1) };
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); //lowercase
		return ext;
	}
}