#pragma once
#include "Graphics/Models/Model.h"
#include "Graphics/Textures/Texture.h"
#include "Graphics/Utilities/Data.h"

//#include <unordered_map>

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
		PbrMaterial data{};
		AssetID albedoMapID{ EMPTY_ASSET };
		AssetID normalMapID{ EMPTY_ASSET };
		AssetID roughnessMapID{ EMPTY_ASSET };
		AssetID metallicMapID{ EMPTY_ASSET };
		AssetID occlusionMapID{ EMPTY_ASSET };
		AssetID emissiveMapID{ EMPTY_ASSET };
	};

	struct TextureAsset : Asset {
		Texture data{};
		bool isHDR{};
		bool isFlipY{true};
	};

	struct SkyboxAsset : Asset
	{
		Skybox data{};
		Texture envMap{};
		int32_t size{ 2048 };
		bool isHDR{ true };
		bool isFlipY{ true };
	};

	struct ModelAsset : Asset {
		Model3D data{};
		bool hasJoints{};
	};

	//TODO(other uncompleted/custom types):
	struct ScriptAsset : Asset {

	};
	struct SceneAsset : Asset {

	};

	using SharedAsset = std::shared_ptr<Asset>;
	using AssetMap = std::unordered_map<AssetID, SharedAsset>;

	struct AssetRegistry {
		//inits the registry, prevents nullptr asset
		BOOM_INLINE AssetRegistry() {
			AddEmpty<MaterialAsset>();
			AddEmpty<TextureAsset>();
			AddEmpty<SkyboxAsset>();
			AddEmpty<ModelAsset>();

			AddEmpty<ScriptAsset>();
			AddEmpty<SceneAsset>();
		}

		//tries to get asset by its defined type
		template <class T>
		BOOM_INLINE T& Get(AssetID uid) {
			const uint32_t type{ TypeID<T>() };
			if (registry[type].count(uid)) {
				return (T&)(*registry[type][uid]);
			}
			return (T&)(*registry[type][EMPTY_ASSET]);
		}

		//loops through all assets and
		//runs func(shared_ptr<Asset>.get()) on non EMPTY_ASSET
		template <class F>
		BOOM_INLINE void View(F&& func) {
			for (auto& [_, assetMap] : registry) {
				for (auto& [uid, asset] : assetMap) {
					if (uid != EMPTY_ASSET) {
						func(asset.get()); //shared_ptr<Asset>
					}
				}
			}
		}

		//collection of asset
		template <class T>
		BOOM_INLINE auto& GetMap() {
			return registry[TypeID<T>()];
		}

		BOOM_INLINE void Clear() {
			registry.clear();
		}

	public: //custom and specific assets
		//file path starts from Textures folder
		BOOM_INLINE auto AddSkybox(
			AssetID uid,
			std::string const& path,
			int32_t size,
			bool isHDR = true,
			bool isFlipY = true)
		{
			auto asset{ std::make_shared<SkyboxAsset>() };
			asset->type = AssetType::SKYBOX;
			asset->envMap = std::make_shared<Texture2D>(path, isFlipY, isHDR);
			asset->isHDR = isHDR;
			asset->isFlipY = isFlipY;
			asset->size = size;
			Add(uid, path, asset);
			return asset;
		}
		//file path starts from Textures folder
		BOOM_INLINE auto AddTexture(
			AssetID uid,
			std::string const& path,
			bool isHDR = false,
			bool isFlipY = true)
		{
			auto asset{ std::make_shared<TextureAsset>() };
			asset->type = AssetType::TEXTURE;
			asset->data = std::make_shared<Texture2D>(path, isFlipY, isHDR);
			asset->isFlipY = isFlipY;
			asset->isHDR = isHDR;
			Add(uid, path, asset);
			return asset;
		}
		BOOM_INLINE auto AddModel(
			AssetID uid,
			std::string const& path,
			bool hasJoints = false)
		{
			auto asset{ std::make_shared<ModelAsset>() };
			asset->type = AssetType::MODEL;
			asset->hasJoints = hasJoints;
			if (hasJoints) {
				asset->data = std::make_shared<SkeletalModel>(path);
			}
			else {
				asset->data = std::make_shared<StaticModel>(path);
			}
			Add(uid, path, asset);
			return asset;
		}
		BOOM_INLINE auto AddMaterial(AssetID uid, std::string const& path, std::array<AssetID, 6> uidMaps = {}) {
			auto asset{ std::make_shared<MaterialAsset>() };
			asset->type = AssetType::MATERIAL;
			asset->albedoMapID = uidMaps[0];
			asset->normalMapID = uidMaps[1];
			asset->roughnessMapID = uidMaps[2];
			asset->metallicMapID = uidMaps[3];
			asset->occlusionMapID = uidMaps[4];
			asset->emissiveMapID = uidMaps[5];
			Add(uid, path, asset);
			return asset;
		}
		BOOM_INLINE auto AddScript(AssetID uid, std::string const& path) {
			//std::string fullPath{ CONSTANTS::TEXTURES_LOCATION.data() + path };
			auto asset{ std::make_shared<ScriptAsset>() };
			asset->type = AssetType::SCRIPT;
			Add(uid, path, asset);
			return asset;
		}
		BOOM_INLINE auto AddScene(AssetID uid, std::string const& path) {
			auto asset{ std::make_shared<SceneAsset>() };
			asset->type = AssetType::SCENE;
			Add(uid, path, asset);
			return asset;
		}

	private:
		template <class T>
		BOOM_INLINE void Add(
			AssetID uid, 
			std::string const& source, 
			std::shared_ptr<T>& asset) 
		{
			asset->uid = uid;
			asset->source = source;
			std::filesystem::path path{ source };
			asset->name = path.stem().string();
			registry[TypeID<T>()][asset->uid] = asset;
		}
		template <class T>
		BOOM_INLINE void AddEmpty() {
			registry[TypeID<T>()][EMPTY_ASSET] = std::make_shared<T>();
		}

	private:
		std::unordered_map<uint32_t, AssetMap> registry;
	};

}