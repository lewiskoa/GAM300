#pragma once

#include "Graphics/Textures/Texture.h"
#include "../Application/Interface.h"
#pragma warning(push)
#pragma warning(disable : 4244 4267 4458 4100 5054 4189 26819 6262 26495) //library warnings ignored
#include <gli/gli.hpp>
#include <compressonator.h>
#pragma warning(pop)

namespace Boom {
	//called once to compress all TextureAssets
	//MUST BE CALLED WITHIN TRY CATCH BLOCK
	struct BOOM_API CompressAllTextures {

		//called once to compress all TextureAssets
		//MUST BE CALLED WITHIN TRY CATCH BLOCK
		//callback must be of type bool(float, size_t, size_t)
		CompressAllTextures(AssetMap textureMap, std::string_view const& outputPath);
		operator bool() const noexcept { return success; }

	private:
		void SetKernelOpt(KernelOptions& kOpt, Texture2D const& texRef);
		std::string GetExtension(std::string const& filename);
		CMP_FORMAT destFormat{ CMP_FORMAT_BC7 };
		bool Callback(float, size_t, size_t);

		bool success{};
	};
}