#pragma once
#include "Graphics/Models/Model.h"
#include "Graphics/Textures/Texture.h"
#include "Graphics/Utilities/Data.h"

namespace Boom {
	using AssetID = uint64_t;
	const AssetID EMPTY_ASSET = 0u;

	enum class AssetType : uint8_t {
		UNKNOWN = 0u,
		MATERIAL,
		TEXTURE,
		SKYBOX,
		SCRIPT,
		SCENE,
		MODEL
	};

	struct Asset {
		AssetID uid{ EMPTY_ASSET };
		std::string source{};
		std::string name{};
		AssetType type{};
	};

	struct MaterialAsset : Asset {
		AssetID RoughnessMap{ EMPTY_ASSET };
		AssetID OcclusionMap{ EMPTY_ASSET };
		AssetID MetallicMap{ EMPTY_ASSET };
		AssetID EmissiveMap{ EMPTY_ASSET };
		AssetID AlbedoMap{ EMPTY_ASSET };
		AssetID NormalMap{ EMPTY_ASSET };
		PbrMaterial data;
	};

	struct TextureAsset : Asset {
		bool isHDR{};
		bool isFlipV{true};
		Texture2D data;
	};

	struct SkyboxAsset : Asset
	{
		int32_t size{ 2048 };
		bool IsHDR{ true };
		bool FlipV{ true };
		Texture2D envMap;
		Skybox data;
	};

	struct ModelAsset : Asset {
		bool hasJoints{};
		Model3D data;
	};

	using SharedAsset = std::shared_ptr<Asset>;
	using AssetMap = std::unordered_map<AssetID, SharedAsset>;
}