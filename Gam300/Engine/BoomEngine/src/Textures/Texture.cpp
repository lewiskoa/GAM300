#include "Core.h"
#include "Graphics/Textures/Texture.h"

#pragma warning(push)
#pragma warning(disable : 4244 4267) //stb_image's int to short, int to unsigned char warnings
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)

namespace Boom {
	Texture2D::Texture2D(std::string const& filename)
		: Texture2D()
	{
		Load(filename);
	}
	Texture2D::~Texture2D() {
		glDeleteTextures(1, &id);
	}

	bool Texture2D::Load(std::string const& filename) {
		//flip y axis (needed operation for many image types)
		stbi_set_flip_vertically_on_load(true);

		//texture data
		uint8_t* pixels{ stbi_load(filename.c_str(), &width, &height, nullptr, 4) };
		if (pixels == nullptr) {
			BOOM_ERROR("failed Texture2D::Load({})", filename);
			return false;
		}

		//texture buffers
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		stbi_image_free(pixels);

		//options and optimization
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
		return true;
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
}