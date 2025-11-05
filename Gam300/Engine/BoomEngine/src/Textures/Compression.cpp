#include "Core.h"
#include "GlobalConstants.h"
#include "Auxiliaries/Assets.h"

#include "Graphics/Textures/Compression.h"

namespace Boom {
	CompressAllTextures::CompressAllTextures(AssetMap textureMap, std::string_view const& outputPath)
	{
		if (textureMap.empty()) return;

		CMP_InitFramework();

		//create in case of missing directories
		std::filesystem::create_directories(outputPath);
		
		for (auto const& [id, sharedAsset] : textureMap) {
			if (id == 0) continue;
			TextureAsset* asset = dynamic_cast<TextureAsset*>(sharedAsset.get());
			if (!asset) continue;

			//already compressed simple copy paste (textures that are already compressed or not to be compressed)
			if (GetExtension(asset->source) == "dds" || !asset->data->isCompileAsCompressed) {
				std::error_code ec;
				if (!std::filesystem::copy_file(asset->source, std::filesystem::path{ outputPath } / asset->name, std::filesystem::copy_options::overwrite_existing, ec)) {
					std::string dodo{ ec.message() };
					std::string destPathString{ (std::filesystem::path{ outputPath } / asset->name).generic_string() };
					BOOM_ERROR("copy failed - error_code:{}, source:{}, dest:{}", dodo, asset->source, destPathString);
				}
				continue;
			}
			CMP_MipSet mipIn;
			memset(&mipIn, 0, sizeof(CMP_MipSet));
			CMP_ERROR status{ CMP_LoadTexture(asset->source.c_str(), &mipIn) };
			if (status != CMP_OK) {
				throw std::exception((asset->source + "_CMP_LoadTexture() - error_code: " + std::to_string(status)).c_str());
			}

			if (mipIn.m_nMipLevels <= 1) {
				CMP_INT minMipSize{ CMP_CalcMinMipSize(mipIn.m_nHeight, mipIn.m_nWidth, asset->data->mipLevel) };
				CMP_GenerateMIPLevels(&mipIn, minMipSize);
			}

			KernelOptions kOpt{};
			SetKernelOpt(kOpt, *asset->data);

			BOOM_INFO("======Compressing {}({}-{})========", asset->name, asset->uid, asset->source);
			CMP_MipSet mipOut;
			memset(&mipOut, 0, sizeof(CMP_MipSet));
			status = CMP_ProcessTexture(&mipIn, &mipOut, kOpt, Callback);
			if (status != CMP_OK) {
				CMP_FreeMipSet(&mipIn);
				throw std::exception((asset->source + "_CMP_ProcessTexture() - error_code: " + std::to_string(status)).c_str());
			}

			std::string fullPath{ outputPath.data() + ("/" + std::filesystem::path(asset->name).stem().string()) + ".dds"};
			status = CMP_SaveTexture(fullPath.c_str(), &mipOut);
			BOOM_INFO("Saving Texture...");

			//cleanup
			CMP_FreeMipSet(&mipIn);
			CMP_FreeMipSet(&mipOut);

			if (status != CMP_OK) {
				throw std::exception((asset->source + "_CMP_SaveTexture() - error_code: " + std::to_string(status)).c_str());
			}
		}

		success = true;
	}

	void CompressAllTextures::SetKernelOpt(KernelOptions& kOpt, Texture2D const& texRef) {
		kOpt.format = destFormat;
		kOpt.fquality = texRef.quality;
		kOpt.useSRGBFrames = texRef.isGamma;
		kOpt.threads = 2;
		kOpt.encodeWith = CMP_HPC;

		//set bc15 props //TODO modify to descriptor reading
		kOpt.bc15.useAlphaThreshold = true;
		kOpt.bc15.alphaThreshold = texRef.alphaThreshold;
		kOpt.bc15.useChannelWeights = true;
		kOpt.bc15.channelWeights[0] = 0.3086f;
		kOpt.bc15.channelWeights[1] = 0.6094f;
		kOpt.bc15.channelWeights[2] = 0.0820f;
	}
	
	bool CompressAllTextures::Callback(float, size_t, size_t) {
		//BOOM_INFO("{}%", p);
		return false;
	}

	std::string CompressAllTextures::GetExtension(std::string const& filename) {
		uint32_t pos{ (uint32_t)filename.find_last_of('.') };
		if (pos == std::string::npos || pos == filename.length() - 1) {
			return ""; //no extension
		}
		std::string ext{ filename.substr(pos + 1) };
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); //lowercase
		return ext;
	}
}