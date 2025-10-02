#pragma once
#include "ECS/ECS.hpp"
#include "Common/YAML.h"
#include "Auxiliaries/ComponentSerializer.h"
#include "Auxiliaries/AssetSerializer.h"

namespace Boom
{
    struct DataSerializer
    {
        // ===== ENTITY SERIALIZATION =====
        BOOM_INLINE void Serialize(EntityRegistry& scene, const std::string& path)
        {
            YAML::Emitter emitter;
            emitter << YAML::BeginMap;
            emitter << YAML::Key << "ENTITIES" << YAML::Value << YAML::BeginSeq;

            for (auto entt : scene.view<entt::entity>())
            {
                emitter << YAML::BeginMap;
                ComponentSerializer::SerializeAll(emitter, scene, entt);
                emitter << YAML::EndMap;
            }

            emitter << YAML::EndSeq;
            emitter << YAML::EndMap;
            std::ofstream filepath(path);
            filepath << emitter.c_str();
        }

        BOOM_INLINE void Deserialize(EntityRegistry& scene, AssetRegistry& assets, const std::string& path)
        {
            auto root = YAML::LoadFile(path);
            const auto& nodes = root["ENTITIES"];

            [[maybe_unused]] auto _ = scene.create();

            for (auto& node : nodes)
            {
                EntityID entity = scene.create();
                ComponentSerializer::DeserializeAll(node, scene, entity, assets);
            }
        }

        // ===== ASSET SERIALIZATION =====
        BOOM_INLINE void Serialize(AssetRegistry& registry, const std::string& path)
        {
            YAML::Emitter emitter;
            emitter << YAML::BeginMap;
            emitter << YAML::Key << "ASSETS" << YAML::Value << YAML::BeginSeq;

            registry.View([&](Asset* asset)
                {
                    std::string typeName(magic_enum::enum_name(asset->type));
                    emitter << YAML::BeginMap;
                    emitter << YAML::Key << "Type" << YAML::Value << typeName;
                    emitter << YAML::Key << "UID" << YAML::Value << asset->uid;
                    emitter << YAML::Key << "Name" << YAML::Value << asset->name;
                    emitter << YAML::Key << "Source" << YAML::Value << asset->source;

                    // Serialize properties using registered function
                    AssetSerializer::SerializeProperties(emitter, asset);

                    emitter << YAML::EndMap;
                });

            emitter << YAML::EndSeq;
            emitter << YAML::EndMap;
            std::ofstream filepath(path);
            filepath << emitter.c_str();
        }

        BOOM_INLINE void Deserialize(AssetRegistry& registry, const std::string& path)
        {
            auto root = YAML::LoadFile(path);
            const auto& nodes = root["ASSETS"];

            for (auto& node : nodes)
            {
                auto props = node["Properties"];
                auto uid = node["UID"].as<AssetID>();
                auto name = node["Name"].as<std::string>();
                auto source = node["Source"].as<std::string>();
                auto typeName = node["Type"].as<std::string>();
                auto opt = magic_enum::enum_cast<AssetType>(typeName);
                auto type = opt.has_value() ? opt.value() : AssetType::UNKNOWN;

                BOOM_INFO("[Deserialize] Processing asset UID={}, Type={}", uid, typeName);

                // Deserialize using registered function
                Asset* asset = AssetSerializer::Deserialize(registry, type, uid, source, props);

                if (asset)
                {
                    asset->source = source;
                    asset->name = name;
                }
                else
                {
                    BOOM_ERROR("Failed to deserialize asset: invalid type!");
                }
            }
        }
    };
}