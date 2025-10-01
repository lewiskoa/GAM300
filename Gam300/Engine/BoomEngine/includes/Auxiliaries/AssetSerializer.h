#pragma once
#include "Core.h"
#include "Auxiliaries/Assets.h"
#include "Common/YAML.h"


namespace Boom
{
    class AssetSerializer
    {
    public:
        using SerializePropsFunc = std::function<void(YAML::Emitter&, Asset*)>;
        using DeserializeFunc = std::function<Asset* (AssetRegistry&, AssetID, const std::string&, const YAML::Node&)>;

        BOOM_INLINE static void Register(AssetType type, SerializePropsFunc ser, DeserializeFunc deser)
        {
            GetRegistry()[type] = { ser, deser };
        }

        BOOM_INLINE static void SerializeProperties(YAML::Emitter& e, Asset* asset)
        {
            auto it = GetRegistry().find(asset->type);
            if (it != GetRegistry().end() && it->second.serializeProps)
            {
                it->second.serializeProps(e, asset);
            }
        }

        BOOM_INLINE static Asset* Deserialize(AssetRegistry& registry, AssetType type, AssetID uid,
            const std::string& source, const YAML::Node& props)
        {
            auto it = GetRegistry().find(type);
            if (it != GetRegistry().end())
            {
                return it->second.deserialize(registry, uid, source, props);
            }
            return nullptr;
        }

    private:
        struct Entry { SerializePropsFunc serializeProps; DeserializeFunc deserialize; };

        BOOM_INLINE static std::unordered_map<AssetType, Entry>& GetRegistry()
        {
            static std::unordered_map<AssetType, Entry> registry;
            return registry;
        }
    };

    BOOM_INLINE void RegisterAllAssets()
    {
        // MATERIAL
        AssetSerializer::Register(AssetType::MATERIAL,
            [](YAML::Emitter& e, Asset* asset) {
                auto mtl = static_cast<MaterialAsset*>(asset);
                e << "Properties" << YAML::BeginMap;
                e << YAML::Key << "AlbedoMap" << YAML::Value << mtl->albedoMapID;
                e << YAML::Key << "NormalMap" << YAML::Value << mtl->normalMapID;
                e << YAML::Key << "RoughnessMap" << YAML::Value << mtl->roughnessMapID;
                e << YAML::Key << "MetallicMap" << YAML::Value << mtl->metallicMapID;
                e << YAML::Key << "OcclusionMap" << YAML::Value << mtl->occlusionMapID;
                e << YAML::Key << "EmissiveMap" << YAML::Value << mtl->emissiveMapID;
                e << YAML::Key << "Albedo" << YAML::Value << mtl->data.albedo;
                e << YAML::Key << "Metallic" << YAML::Value << mtl->data.metallic;
                e << YAML::Key << "Roughness" << YAML::Value << mtl->data.roughness;
                e << YAML::Key << "Occlusion" << YAML::Value << mtl->data.occlusion;
                e << YAML::Key << "Emissive" << YAML::Value << mtl->data.emissive;
                e << YAML::EndMap;
            },
            [](AssetRegistry& reg, AssetID uid, const std::string& src, const YAML::Node& props) -> Asset* {
                auto mtl = reg.AddMaterial(uid, src);
                mtl->albedoMapID = props["AlbedoMap"].as<AssetID>();
                mtl->normalMapID = props["NormalMap"].as<AssetID>();
                mtl->roughnessMapID = props["RoughnessMap"].as<AssetID>();
                mtl->metallicMapID = props["MetallicMap"].as<AssetID>();
                mtl->occlusionMapID = props["OcclusionMap"].as<AssetID>();
                mtl->emissiveMapID = props["EmissiveMap"].as<AssetID>();
                mtl->data.albedo = props["Albedo"].as<glm::vec3>();
                mtl->data.roughness = props["Roughness"].as<float>();
                mtl->data.metallic = props["Metallic"].as<float>();
                mtl->data.occlusion = props["Occlusion"].as<float>();
                mtl->data.emissive = props["Emissive"].as<glm::vec3>();
                return static_cast<Asset*>(mtl.get());
            });

        // TEXTURE
        AssetSerializer::Register(AssetType::TEXTURE,
            [](YAML::Emitter& e, Asset* asset) {
                auto tex = static_cast<TextureAsset*>(asset);
                e << "Properties" << YAML::BeginMap;
                e << YAML::Key << "IsHDR" << YAML::Value << tex->isHDR;
                e << YAML::Key << "IsFlipY" << YAML::Value << tex->isFlipY;
                e << YAML::EndMap;
            },
            [](AssetRegistry& reg, AssetID uid, const std::string& src, const YAML::Node& props) -> Asset* {
                bool isHDR = props["IsHDR"].as<bool>();
                bool isFlipY = props["IsFlipY"].as<bool>();
                return (Asset*)reg.AddTexture(uid, src, isHDR, isFlipY).get();
            });

        // SKYBOX
        AssetSerializer::Register(AssetType::SKYBOX,
            [](YAML::Emitter& e, Asset* asset) {
                auto skybox = static_cast<SkyboxAsset*>(asset);
                e << "Properties" << YAML::BeginMap;
                e << YAML::Key << "Size" << YAML::Value << skybox->size;
                e << YAML::Key << "IsHDR" << YAML::Value << skybox->isHDR;
                e << YAML::Key << "IsFlipY" << YAML::Value << skybox->isFlipY;
                e << YAML::EndMap;
            },
            [](AssetRegistry& reg, AssetID uid, const std::string& src, const YAML::Node& props) -> Asset* {
                int32_t size = props["Size"].as<int32_t>();
                bool isHDR = props["IsHDR"].as<bool>();
                bool isFlipY = props["IsFlipY"].as<bool>();
                return (Asset*)reg.AddSkybox(uid, src, size, isHDR, isFlipY).get();
            });

        // MODEL
        AssetSerializer::Register(AssetType::MODEL,
            [](YAML::Emitter& e, Asset* asset) {
                auto model = static_cast<ModelAsset*>(asset);
                e << "Properties" << YAML::BeginMap;
                e << YAML::Key << "HasJoints" << YAML::Value << model->hasJoints;
                e << YAML::EndMap;
            },
            [](AssetRegistry& reg, AssetID uid, const std::string& src, const YAML::Node& props) -> Asset* {
                bool hasJoints = props["HasJoints"].as<bool>();
                return (Asset*)reg.AddModel(uid, src, hasJoints).get();
            });

        // SCENE (no properties)
        AssetSerializer::Register(AssetType::SCENE,
            [](YAML::Emitter&, Asset*) { /* No properties */ },
            [](AssetRegistry& reg, AssetID uid, const std::string& src, const YAML::Node&) -> Asset* {
                return (Asset*)reg.AddScene(uid, src).get();
            });
    }
}