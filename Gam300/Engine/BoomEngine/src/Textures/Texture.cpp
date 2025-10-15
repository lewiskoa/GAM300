#include "Core.h"
#include "Graphics/Textures/Texture.h"
#include "GlobalConstants.h"

#pragma warning(push)
#pragma warning(disable : 4244 4267 4458 4100 5054 4189) //library warnings ignored
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <gli/gli.hpp>
#pragma warning(pop)

#include <compressonator.h>

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

	void Texture2D::CompressTextureForEditor(std::string const& inputPng, std::string const& fullPath, bool forceAlpha) {
		try {
			//validation
			if (!std::filesystem::exists(inputPng)) {
				throw std::runtime_error("Input PNG non-existant: " + inputPng);
			}

			//create out dir if missing
			std::filesystem::path outDir{ std::filesystem::path(fullPath).parent_path() };
			if (!outDir.empty() && !std::filesystem::exists(outDir)) {
				std::filesystem::create_directories(outDir);
			}

			CompressTexture(inputPng, fullPath, forceAlpha);
		}
		catch (std::exception const& e) {
			BOOM_ERROR("Error Compressing Texture: {}", e.what());
			//std::exit(0);
		}
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
			
			//temporary testing
			CompressTexture(filename.c_str(), (filename + ".dds").c_str(), true);
			std::exit(0);
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

	void Texture2D::CompressTexture(std::string const& inputPng, std::string const& outputDDS, bool forceAlpha) {
		CMP_InitFramework();

		stbi_set_flip_vertically_on_load(true);
		//texture data
		int channels{};
		unsigned char* pixels{ stbi_load(inputPng.c_str(), &width, &height, &channels, 4) };

		//check for alpha
		bool hasAlpha{};
		if (channels == 4) {
			for (int i{ 3 }; i < width * height * 4; i += 4) {
				if (pixels[i] < 255) {
					hasAlpha = true;
					break;
				}
			}
		}

		CMP_FORMAT texFormat{ forceAlpha || hasAlpha ? CMP_FORMAT_BC7 : CMP_FORMAT_BC1 };

		//set up source and target texture info
		CMP_Texture srcTex{};
		CMP_Texture dstTex{};

		dstTex.dwSize = srcTex.dwSize = sizeof(CMP_Texture);
		dstTex.dwWidth = srcTex.dwWidth = width;
		dstTex.dwHeight = srcTex.dwHeight = height;
		srcTex.dwPitch = width * 4; //png = RGBA, 4 bytes per pixel
		srcTex.format = CMP_FORMAT_RGBA_8888; //8bits depth per channel as per png format
		srcTex.dwDataSize = width * height * 4;
		srcTex.pData = pixels;
		dstTex.format = texFormat;

		//calculate size for compressed data
		CMP_INT blockSize{ texFormat == CMP_FORMAT_BC7 ? 16 : 8 };
		CMP_INT blocksX{ (width + 3) / 4 }; //4x4 pixels blocks
		CMP_INT blocksY{ (height + 3) / 4 };
		dstTex.dwDataSize = blocksX * blocksY * blockSize;
		dstTex.pData = new CMP_BYTE[dstTex.dwDataSize];
		if (!dstTex.pData) {
			stbi_image_free(pixels);
			throw std::runtime_error("Memory allocation failed for compressed texture.");
		}

		//setup options
		CMP_CompressOptions options{};
		options.dwSize = sizeof(CMP_CompressOptions);
		options.fquality = 1.f; //range[0.f, 1.f] default: 0.5f
		//options.dwnumThreads = 8; //to be adjusted based on system, default is auto

		CMP_ERROR status{ CMP_ConvertTexture(&srcTex, &dstTex, &options, nullptr) };
		if (status != CMP_OK) {
			delete[] dstTex.pData;
			dstTex.pData = nullptr;
			stbi_image_free(pixels);
			throw std::runtime_error("1CMP_ERROR failed with code(refer to compression.h): " + std::to_string(status));
		}
		
		CMP_MipSet mipSet{};
		status = CMP_CreateMipSet(&mipSet, dstTex.dwWidth, dstTex.dwHeight, 1, CF_8bit, CMP_TextureType::TT_2D);
		if (status != CMP_OK) {
			delete dstTex.pData;
			dstTex.pData = nullptr;
			stbi_image_free(pixels);
			throw std::runtime_error("2CMP_ERROR failed with code(refer to compression.h): " + std::to_string(status));
		}
		mipSet.m_format = texFormat;
		mipSet.m_nMipLevels = 1; //single
		mipSet.m_pMipLevelTable = new CMP_MipLevel*[1];
		mipSet.m_pMipLevelTable[0] = new CMP_MipLevel;
		mipSet.m_pMipLevelTable[0]->m_nWidth = width;
		mipSet.m_pMipLevelTable[0]->m_nHeight = height;
		mipSet.m_pMipLevelTable[0]->m_dwLinearSize = dstTex.dwDataSize;
		mipSet.m_pMipLevelTable[0]->m_pbData = dstTex.pData;

		//save as .dds file in output directory
		status = CMP_SaveTexture(outputDDS.c_str(), &mipSet);
		if (status != CMP_OK) {
			delete dstTex.pData;
			dstTex.pData = nullptr;
			stbi_image_free(pixels);
			throw std::runtime_error("3CMP_ERROR failed with code(refer to compression.h): " + std::to_string(status));
		}

		CMP_FreeMipSet(&mipSet);
		delete[] dstTex.pData;
		stbi_image_free(pixels);
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