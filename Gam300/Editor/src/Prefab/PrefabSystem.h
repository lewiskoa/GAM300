#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include "ECS/ECS.hpp" 

namespace Boom {

    struct PrefabSystem {
        // Load a prefab file into the given registry
        static AssetID FindModelIDByName(AssetRegistry* assets, const std::string& name) {
            AssetID id = EMPTY_ASSET;
            assets->View([&](Asset* a) {
                if (a->type == AssetType::MODEL && a->name == name)
                    id = a->uid;
                });
            return id;
        }

        static AssetID FindMaterialIDByName(AssetRegistry* assets, const std::string& name) {
            AssetID id = EMPTY_ASSET;
            assets->View([&](Asset* a) {
                if (a->type == AssetType::MATERIAL && a->name == name)
                    id = a->uid;
                });
            return id;
        }


        static entt::entity InstantiatePrefab(entt::registry& registry, AssetRegistry* assets, const std::string& path) {
            std::ifstream f(path);
            if (!f.is_open()) return entt::null;

            nlohmann::json prefabJson = nlohmann::json::parse(f);
            entt::entity newEntity = registry.create();

            const auto& components = prefabJson["components"];
            bool hasTransform = false;

            for (auto& [componentName, componentData] : components.items()) {

                if (componentName == "TransformComponent") {
                    registry.emplace<TransformComponent>(newEntity).deserialize(componentData);
                    hasTransform = true;
                }
                else if (componentName == "InfoComponent") {
                    registry.emplace<InfoComponent>(newEntity).deserialize(componentData);
                }
                else if (componentName == "ModelComponent") {
                    auto& mc = registry.emplace<ModelComponent>(newEntity);

                    if (componentData.contains("modelName"))
                        mc.modelName = componentData["modelName"].get<std::string>();
                    if (componentData.contains("materialName"))
                        mc.materialName = componentData["materialName"].get<std::string>();

                    // Lookup IDs dynamically at runtime
                    mc.modelID = FindModelIDByName(m_Context->assets, mc.modelName);
                    mc.materialID = FindMaterialIDByName(m_Context->assets, mc.materialName);

                    if (mc.modelID == EMPTY_ASSET)
                        std::cout << "[Prefab] Warning: Model '" << mc.modelName << "' not found!\n";
                    if (mc.materialID == EMPTY_ASSET)
                        std::cout << "[Prefab] Warning: Material '" << mc.materialName << "' not found!\n";
                }

            }

            // Ensure a default transform exists so prefab is visible
            if (!hasTransform) {
                auto& tc = registry.emplace<TransformComponent>(newEntity);
                tc.transform.translate = glm::vec3(0.f, 0.f, 5.f); // In front of camera
                tc.transform.scale = glm::vec3(1.f);               // Normal size
                tc.transform.rotate = glm::vec3(0.f);              // No rotation
            }

            std::cout << "Spawned prefab from: " << path << " (entity ID " << (uint32_t)newEntity << ")\n";

            return newEntity;
        }


    };

}
