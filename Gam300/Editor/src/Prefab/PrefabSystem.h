#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include "ECS/ECS.hpp"
#include "Auxiliaries/Assets.h"  // AssetRegistry

namespace Boom {

    struct PrefabSystem {

        static entt::entity InstantiatePrefab(entt::registry& registry,
            AssetRegistry& assets,
            const std::string& path)
        {
            std::ifstream f(path);
            if (!f.is_open()) {
                std::cout << "[ERROR] Could not open prefab: " << path << "\n";
                return entt::null;
            }

            nlohmann::json prefabJson;
            try {
                prefabJson = nlohmann::json::parse(f);
            }
            catch (const std::exception& e) {
                std::cout << "[ERROR] Failed to parse prefab JSON: " << e.what() << "\n";
                return entt::null;
            }

            Entity newEntity{ &registry };

            const auto& components = prefabJson["components"];
            for (auto& [componentName, componentData] : components.items()) {

                if (componentName == "InfoComponent") {
                    auto& info{ newEntity.Attach<InfoComponent>() };
                    info.name = componentData.at("name");
                }
                else if (componentName == "TransformComponent") {
                    newEntity.Attach<TransformComponent>().deserialize(componentData);
                }
                else if (componentName == "ModelComponent") {
                    auto& mc = newEntity.Attach<ModelComponent>();

                    // Load model either by name or path
                    if (componentData.contains("modelName")) {
                        std::string modelName = componentData["modelName"].get<std::string>();
                        AssetID modelID = assets.FindModelByName(modelName);
                        if (modelID != EMPTY_ASSET) {
                            mc.modelID = modelID;
                            std::cout << "[DEBUG] Assigned model '" << modelName
                                << "' ID " << modelID << "\n";
                        }
                        else {
                            mc.modelID = EMPTY_ASSET;
                            std::cout << "[WARN] Model not found: " << modelName << "\n";
                        }
                    }
                    else if (componentData.contains("modelPath")) {
                        std::string modelPath = componentData["modelPath"].get<std::string>();
                        auto assetPtr = assets.AddModel(RandomU64(), modelPath, false); // skeletal=false for now
                        if (assetPtr) {
                            mc.modelID = assetPtr->uid;
                            std::cout << "[DEBUG] Loaded model from path: " << modelPath << "\n";
                        }
                        else {
                            mc.modelID = EMPTY_ASSET;
                            std::cout << "[WARN] Failed to load model from path: " << modelPath << "\n";
                        }
                    }

                    // Skip material to avoid crashes
                    mc.materialID = EMPTY_ASSET;
                }
            }

            std::cout << "[DEBUG] Spawned prefab entity: " << (uint32_t)newEntity
                << " from " << path << "\n";
            return newEntity;
        }

    };

}
