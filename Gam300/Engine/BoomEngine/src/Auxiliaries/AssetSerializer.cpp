#include "Core.h"
#include "Auxiliaries/SerializationRegistry.h"
#include "Auxiliaries/Assets.h"

namespace Boom
{
    void RegisterAllAssetSerializers()
    {
        auto& registry = SerializationRegistry::Instance();

        // === MATERIAL ===
        registry.RegisterAssetSerializer(
            AssetType::MATERIAL,
            // Serialize
            [](YAML::Emitter& e, Asset* asset) {
                auto mtl = static_cast<MaterialAsset*>(asset);
                e << YAML::Key << "Properties" << YAML::Value << YAML::BeginMap;
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
            // Deserialize
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
            }
        );

        // === TEXTURE ===
        registry.RegisterAssetSerializer(
            AssetType::TEXTURE,
            // Serialize
            [](YAML::Emitter& e, Asset* asset) {
                auto tex = static_cast<TextureAsset*>(asset);
                e << YAML::Key << "Properties" << YAML::Value << YAML::BeginMap;
                e << YAML::Key << "IsHDR" << YAML::Value << tex->isHDR;
                e << YAML::Key << "IsFlipY" << YAML::Value << tex->isFlipY;
                e << YAML::EndMap;
            },
            // Deserialize
            [](AssetRegistry& reg, AssetID uid, const std::string& src, const YAML::Node& props) -> Asset* {
                bool isHDR = props["IsHDR"].as<bool>();
                bool isFlipY = props["IsFlipY"].as<bool>();
                return static_cast<Asset*>(reg.AddTexture(uid, src, isHDR, isFlipY).get());
            }
        );

        // === SKYBOX ===
        registry.RegisterAssetSerializer(
            AssetType::SKYBOX,
            // Serialize
            [](YAML::Emitter& e, Asset* asset) {
                auto skybox = static_cast<SkyboxAsset*>(asset);
                e << YAML::Key << "Properties" << YAML::Value << YAML::BeginMap;
                e << YAML::Key << "Size" << YAML::Value << skybox->size;
                e << YAML::Key << "IsHDR" << YAML::Value << skybox->isHDR;
                e << YAML::Key << "IsFlipY" << YAML::Value << skybox->isFlipY;
                e << YAML::EndMap;
            },
            // Deserialize
            [](AssetRegistry& reg, AssetID uid, const std::string& src, const YAML::Node& props) -> Asset* {
                int32_t size = props["Size"].as<int32_t>();
                bool isHDR = props["IsHDR"].as<bool>();
                bool isFlipY = props["IsFlipY"].as<bool>();
                return static_cast<Asset*>(reg.AddSkybox(uid, src, size, isHDR, isFlipY).get());
            }
        );

        // === MODEL ===
        registry.RegisterAssetSerializer(
            AssetType::MODEL,
            // Serialize
            [](YAML::Emitter& e, Asset* asset) {
                auto model = static_cast<ModelAsset*>(asset);
                e << YAML::Key << "Properties" << YAML::Value << YAML::BeginMap;
                e << YAML::Key << "HasJoints" << YAML::Value << model->hasJoints;
                e << YAML::EndMap;
            },
            // Deserialize
            [](AssetRegistry& reg, AssetID uid, const std::string& src, const YAML::Node& props) -> Asset* {
                bool hasJoints = props["HasJoints"].as<bool>();
                return static_cast<Asset*>(reg.AddModel(uid, src, hasJoints).get());
            }
        );

        // === SCENE ===
        registry.RegisterAssetSerializer(
            AssetType::SCENE,
            // Serialize (no properties)
            [](YAML::Emitter&, Asset*) {
                // No properties to serialize
            },
            // Deserialize
            [](AssetRegistry& reg, AssetID uid, const std::string& src, const YAML::Node&) -> Asset* {
                return static_cast<Asset*>(reg.AddScene(uid, src).get());
            }
        );

        BOOM_INFO("[AssetSerializers] All asset serializers registered");
    }
}