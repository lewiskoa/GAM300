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
	Texture2D::Texture2D() : height{}, width{}, id{}
		, quality{ 0.5f }
		, alphaThreshold{ 128 }
		, mipLevel{ 10 }
		, isGamma{ true } 
	{
	}

	Texture2D::Texture2D(std::string filename, bool shouldCompress)
		: Texture2D()
	{
		filename = CONSTANTS::TEXTURES_LOCATION.data() + filename;

		try {
			std::string ext{ GetExtension(filename) };
			if (ext != "dds" && !shouldCompress) {
				LoadUnCompressed(filename);
			}
			else {
				if (ext != "dds") {
					std::string outName{ filename.substr(0, filename.find_last_of('.')) + ".dds" };
					CompressTexture(filename, outName);
					filename = outName;
				}
				LoadCompressed(filename);
			}
		}
		catch (std::exception e) {
			char const* tmp{ e.what() };
			BOOM_ERROR("ERROR_Texture2D: {}", tmp);
		}
	}

	Texture2D::Texture2D(std::string const& inputPngPath, std::string const& outputDDSPath)
		: Texture2D() 
	{
		try {
			//validation
			if (!std::filesystem::exists(inputPngPath)) {
				throw std::runtime_error("Input PNG non-existant: " + inputPngPath);
			}

			//create out dir if missing
			std::filesystem::path outDir{ std::filesystem::path(outputDDSPath).parent_path() };
			if (!outDir.empty() && !std::filesystem::exists(outDir)) {
				std::filesystem::create_directories(outDir);
			}

			CompressTexture(inputPngPath, outputDDSPath);
			LoadCompressed(outputDDSPath);
		}
		catch (std::exception const& e) {
			char const* tmp{ e.what() };
			BOOM_ERROR("ERROR_Texture2D: {}", tmp);
		}
	}
	Texture2D::~Texture2D() {
		if (id != 0) {
			glDeleteTextures(1, &id);
		}
	}

	void Texture2D::LoadUnCompressed(std::string const& filename) {

		//texture data
		void* pixels{ stbi_load(filename.c_str(), &width, &height, nullptr, 4) };

		if (pixels == nullptr) {
			throw std::exception(("stbi_load(" + filename + ") failed.").c_str());
		}

		//texture buffers
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (uint32_t*)pixels);
		stbi_image_free(pixels);

		//options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glGenerateMipmap(GL_TEXTURE_2D); //mipmaps should not be needed for uncompressed anymore (small size)

		glBindTexture(GL_TEXTURE_2D, 0);

		//initial .HDR loading
		//int32_t channels{};
		//pixels = stbi_loadf(filename.c_str(), &width, &height, &channels, 0);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, (float*)pixels);
	}
	void Texture2D::LoadCompressed(std::string const& filename) {
		
		// Load DDS using GLI
		gli::texture texture = gli::load(filename);
		if (texture.empty()) {
			throw std::exception(("gli::load(" + filename + ") failed.").c_str());
		}

		// Ensure it's a 2D texture and DXT1 format
		if (texture.target() != gli::TARGET_2D) {
			throw std::exception("texture.target != TARGET_2D");
		}
		gli::gl gl(gli::gl::PROFILE_GL33); // Adjust for your OpenGL version
		gli::gl::format format = gl.translate(texture.format(), texture.swizzles());

		GLenum internalFormat;
		if (format.Internal == gli::gl::INTERNAL_RGBA_DXT1) { //BC1 or DXT1
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		}
		else if (format.Internal == gli::gl::INTERNAL_RGB_BP_UNORM) { //BC7
			internalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM;
		}
		else if (format.Internal == gli::gl::INTERNAL_RGB_BP_UNSIGNED_FLOAT) { //HDR/EXR
			internalFormat = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
		}
		else {
			throw std::exception(("gli::gl::format UNKNOWN - supported:(BC1/DXT1, BC7, BC6H)"));
		}

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		gli::texture2d tex2D(texture);
		width = (int32_t)tex2D.extent().x;
		height = (int32_t)tex2D.extent().y;

		//load textures according to mipmap levels
		for (size_t level{}; level < tex2D.levels(); ++level) {
			GLsizei mipWidth{ (GLsizei)tex2D.extent(level).x };
			GLsizei mipHeight{ (GLsizei)tex2D.extent(level).y };
			GLsizei compressedSize{ (GLsizei)tex2D.size(level) };
			void const* compressedData{ tex2D.data(0, 0, level) };

			glCompressedTexImage2D(
				GL_TEXTURE_2D,
				(GLint)level,
				internalFormat,
				mipWidth,
				mipHeight,
				0,
				compressedSize,
				compressedData
			);

			if (glGetError() != GL_NO_ERROR) {
				glDeleteTextures(1, &id);
				throw std::exception("glCompressedTexImage2D() upload error.");
			}
		}

		//textures has different options if they have multiple levels of mipmaps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex2D.levels() > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(tex2D.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture2D::CompressTexture(std::string const& inputFile, std::string const& outputDDS) {
		CMP_InitFramework();

		//BC7 is best compression (could be expanded if nesassary)
		CMP_FORMAT destFormat{ CMP_FORMAT_BC7 }; 

		CMP_MipSet mipIn{};
		CMP_ERROR status{ CMP_LoadTexture(inputFile.c_str(), &mipIn) };
		if (status != CMP_OK) {
			throw std::exception(("CMP_LoadTexture() - error_code: " + std::to_string(status)).c_str());
		}

		if (mipIn.m_nMipLevels <= 1) {
			CMP_INT minMipSize{ CMP_CalcMinMipSize(mipIn.m_nHeight, mipIn.m_nWidth, mipLevel) };
			CMP_GenerateMIPLevels(&mipIn, minMipSize);
		}

		KernelOptions kOpt{};
		kOpt.format = destFormat;
		kOpt.fquality = quality;
		kOpt.useSRGBFrames = isGamma;
		kOpt.threads = 0;
		kOpt.encodeWith = CMP_HPC;

		//set bc15 props //TODO modify to descriptor reading
		kOpt.bc15.useAlphaThreshold = true;
		kOpt.bc15.alphaThreshold = alphaThreshold;
		kOpt.bc15.useChannelWeights = true;
		kOpt.bc15.channelWeights[0] = 0.3086f;
		kOpt.bc15.channelWeights[1] = 0.6094f;
		kOpt.bc15.channelWeights[2] = 0.0820f;

		auto ComCallback = [](float fProg, CMP_DWORD_PTR, CMP_DWORD_PTR) -> bool {
			//(void)fProg;
			BOOM_INFO("Compression Progress: {}%", fProg);
			return false;
		};

		BOOM_INFO("======Compressing {}========", inputFile);
		CMP_MipSet mipOut{};
		status = CMP_ProcessTexture(&mipIn, &mipOut, kOpt, ComCallback);
		if (status != CMP_OK) {
			CMP_FreeMipSet(&mipIn);
			throw std::exception(("CMP_ProcessTexture() - error_code: " + std::to_string(status)).c_str());
		}

		status = CMP_SaveTexture(outputDDS.c_str(), &mipOut);

		//cleanup
		CMP_FreeMipSet(&mipIn);
		CMP_FreeMipSet(&mipOut);

		if (status != CMP_OK) {
			throw std::exception(("CMP_SaveTexture() - error_code: " + std::to_string(status)).c_str());
		}
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