#pragma once
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

// Make sure this path is correct for your project structure
#include "ECS/ECS.hpp" 

namespace Boom {

    // Saves an entity's components to a file
    inline void SaveEntityAsPrefab(entt::registry& registry, entt::entity entity, const std::string& name) {
        nlohmann::json prefabJson;
        prefabJson["name"] = name;

        // Add an 'if' block for every component you've made serializable
        if (registry.all_of<TransformComponent>(entity)) {
            registry.get<TransformComponent>(entity).serialize(prefabJson["components"]["TransformComponent"]);
        }
        if (registry.all_of<SoundComponent>(entity)) {
            registry.get<SoundComponent>(entity).serialize(prefabJson["components"]["SoundComponent"]);
        }
        if (registry.all_of<InfoComponent>(entity)) {
            registry.get<InfoComponent>(entity).serialize(prefabJson["components"]["InfoComponent"]);
        }
        // ... add more components here as you make them serializable ...

        // This line creates the file. You might need to adjust the path
        // if your program runs from a different directory than your assets.
        std::ofstream o("assets/prefabs/" + name + ".prefab");
        o << std::setw(4) << prefabJson << std::endl;
    }

    inline entt::entity InstantiatePrefab(entt::registry& registry, const std::string& path) {
        // This part for reading the file stays the same
        std::ifstream f(path);
        if (!f.is_open()) return entt::null;

        nlohmann::json prefabJson = nlohmann::json::parse(f);
        entt::entity newEntity = registry.create();

        // --- THIS IS THE CORRECTED LOOP ---
        const auto& components = prefabJson["components"];
        for (auto& [componentName, componentData] : components.items()) {
            // 'componentName' is now a string (e.g., "TransformComponent")
            // 'componentData' is the JSON object for that component

            if (componentName == "TransformComponent") {
                registry.emplace<TransformComponent>(newEntity).deserialize(componentData);
            }
            else if (componentName == "SoundComponent") {
                registry.emplace<SoundComponent>(newEntity).deserialize(componentData);
            }
            else if (componentName == "InfoComponent") {
                registry.emplace<InfoComponent>(newEntity).deserialize(componentData);
            }
            // ... add more components here ...
        }
        // --- END OF CORRECTION ---

        return newEntity;
    }

} // end namespace