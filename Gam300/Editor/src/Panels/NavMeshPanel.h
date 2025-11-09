#pragma once

#include <functional>
#include <vector>
#include <cstdint>
#include <glm/vec3.hpp>
#include <entt/entity/fwd.hpp>

#include "Core.h"          // AssetID, EMPTY_ASSET, glm, etc.
#include "../src/Recast/RecastBaker.h"
#include "Auxiliaries/Assets.h"

namespace Boom { struct AppContext; struct Application; struct AppInterface; }

namespace EditorUI {

    class Editor;

    class NavmeshPanel {
    public:
        explicit NavmeshPanel(Editor* owner);

        void Render();

        // Optional wiring from Editor (menu toggle, etc.)
        void SetShowFlag(bool* flag) { m_ShowNavmesh = flag; }

        // CPU-side mesh container filled by the fetcher.
        struct MeshData {
            std::vector<glm::vec3> positions;  // model-space; we transform to world
            std::vector<uint32_t>  indices;    // triangles (3*n)
        };

       
   
        
       

        // Output path for the Detour .bin file
        void SetOutputPath(const std::string& path);

        // Access bake config (editable via UI)
        RecastBakeConfig& MutableConfig() { return m_Cfg; }
        const RecastBakeConfig& GetConfig() const { return m_Cfg; }

    private:
        // Gathers triangle soup from entities with ModelComponent + Transform3D.
        RecastBakeInput GatherTriangleSoupFromScene(entt::registry& reg);

        // Owner / plumbing
        Editor* m_Owner = nullptr;
        Boom::AppInterface* m_App = nullptr;  // preferred
        Boom::AppContext* m_Ctx = nullptr;  // cached from m_App
        entt::registry* m_Reg = nullptr;  // cached from Editor

        // UI state
        bool* m_ShowNavmesh = nullptr;
        std::string m_OutPath = "Resources/NavData/solo_navmesh.bin";

        // Bake config (editable)
        RecastBakeConfig m_Cfg{};

        // Mesh fetcher hook (must be provided by host/editor)
       

    };

} // namespace EditorUI
