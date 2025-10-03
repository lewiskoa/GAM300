#pragma once
#include "Core.h"
#include "ECS/ECS.hpp"
#include "Auxiliaries/Assets.h"
#include "Auxiliaries/SerializationRegistry.h"
#include "Common/YAML.h"

namespace Boom
{
    /**
     * Utility functions for working with prefabs
     */
    struct BOOM_API PrefabUtility
    {
        /**
         * Creates a prefab from an existing entity
         * @param registry The entity registry containing the entity
         * @param entity The entity to create a prefab from
         * @return YAML string containing the serialized entity
         */
        static std::string SerializeEntity(EntityRegistry& registry, EntityID entity)
        {
            YAML::Emitter emitter;
            emitter << YAML::BeginMap;

            SerializationRegistry::Instance().SerializeAllComponents(emitter, registry, entity);

            emitter << YAML::EndMap;

            return std::string(emitter.c_str());
        }

        /**
        * Creates a prefab asset from an existing entity and adds it to the asset registry
        */
        static std::shared_ptr<PrefabAsset> CreatePrefabFromEntity(
            AssetRegistry& assets,
            AssetID uid,
            const std::string& name,
            EntityRegistry& entityRegistry,
            EntityID entity)
        {
            auto prefab = assets.AddPrefab(uid, "Prefabs/" + name + ".prefab");
            prefab->name = name;

            // Serialize the entity
            YAML::Emitter emitter;
            emitter << YAML::BeginMap;
            SerializationRegistry::Instance().SerializeAllComponents(emitter, entityRegistry, entity);
            emitter << YAML::EndMap;

            prefab->serializedData = std::string(emitter.c_str());

            BOOM_WARN("[PrefabUtility] Created prefab '{}'", name);
            return prefab;
        }
        /**
         * Saves a prefab asset to disk
         */
        static bool SavePrefab(const PrefabAsset& prefab, const std::string& filepath)
        {
            try
            {
                YAML::Emitter emitter;
                emitter << YAML::BeginMap;
                emitter << YAML::Key << "PrefabName" << YAML::Value << prefab.name;
                emitter << YAML::Key << "UID" << YAML::Value << prefab.uid;
                emitter << YAML::Key << "EntityData" << YAML::Value << YAML::Literal << prefab.serializedData;
                emitter << YAML::EndMap;

                std::ofstream file(filepath);
                if (!file.is_open())
                {
                    BOOM_ERROR("[PrefabUtility] Failed to open file for writing: {}", filepath);
                    return false;
                }

                file << emitter.c_str();
                file.close();

                BOOM_INFO("[PrefabUtility] Saved prefab '{}' to: {}", prefab.name, filepath);
                return true;
            }
            catch (const std::exception& e)
            {
                BOOM_ERROR("[PrefabUtility] Failed to save prefab: {}", e.what());
                return false;
            }
        }

        /**
         * Loads a prefab from disk and adds it to the asset registry
         */
        static AssetID LoadPrefab(AssetRegistry& assets, const std::string& filepath)
        {
            try
            {
                auto root = YAML::LoadFile(filepath);

                AssetID uid = root["UID"].as<AssetID>();
                std::string name = root["PrefabName"].as<std::string>();
                std::string entityData = root["EntityData"].as<std::string>();

                auto prefab = assets.AddPrefab(uid, filepath);
                prefab->name = name;
                prefab->serializedData = entityData;

                BOOM_INFO("[PrefabUtility] Loaded prefab '{}' from: {}", name, filepath);
                return uid;
            }
            catch (const YAML::Exception& e)
            {
                BOOM_ERROR("[PrefabUtility] Failed to load prefab: {}", e.what());
                return EMPTY_ASSET;
            }
        }


        /**
         * Instantiates an entity from a prefab
         * @param registry The entity registry to create the entity in
         * @param assets The asset registry containing the prefab
         * @param prefabID The ID of the prefab asset
         * @return The newly created entity ID
         */
        static EntityID Instantiate(EntityRegistry& registry, AssetRegistry& assets, AssetID prefabID)
        {
            PrefabAsset& prefab = assets.Get<PrefabAsset>(prefabID);

            if (prefab.serializedData.empty())
            {
                BOOM_ERROR("[PrefabUtility] Prefab {} has no serialized data", prefabID);
                return entt::null;
            }

            try
            {
                auto node = YAML::Load(prefab.serializedData);
                EntityID entity = registry.create();

                SerializationRegistry::Instance().DeserializeAllComponents(
                    node, registry, entity, assets
                );

                BOOM_INFO("[PrefabUtility] Instantiated prefab {} as entity {}", prefabID, static_cast<uint32_t>(entity));
                return entity;
            }
            catch (const YAML::Exception& e)
            {
                BOOM_ERROR("[PrefabUtility] Failed to instantiate prefab {}: {}", prefabID, e.what());
                return entt::null;
            }
        }
    };
}