#pragma once
#include "Core.h"
#include "Auxiliaries/Assets.h"
#include "ECS/ECS.hpp"
#include "Common/YAML.h"
#include <functional>
#include <memory>

namespace Boom
{
    // Forward declarations
    struct Asset;
    struct AssetRegistry;


    // Serialization function signatures
    using AssetSerializePropsFunc = std::function<void(YAML::Emitter&, Asset*)>;
    using AssetDeserializeFunc = std::function<Asset* (AssetRegistry&, AssetID, const std::string&, const YAML::Node&)>;

    using ComponentSerializeFunc = std::function<void(YAML::Emitter&, EntityRegistry&, EntityID)>;
    using ComponentDeserializeFunc = std::function<void(const YAML::Node&, EntityRegistry&, EntityID, AssetRegistry&)>;

    /**
     * Central registry for all serialization/deserialization functions.
     * This class ensures proper DLL boundary handling and single instance semantics.
     */
    class BOOM_API SerializationRegistry
    {
    public:
        // Singleton access - instance lives in DLL space
        static SerializationRegistry& Instance();

        // === ASSET SERIALIZATION ===
        void RegisterAssetSerializer(
            AssetType type,
            AssetSerializePropsFunc serializeFunc,
            AssetDeserializeFunc deserializeFunc
        );

        void SerializeAssetProperties(YAML::Emitter& emitter, Asset* asset);

        Asset* DeserializeAsset(
            AssetRegistry& registry,
            AssetType type,
            AssetID uid,
            const std::string& source,
            const YAML::Node& properties
        );

        // === COMPONENT SERIALIZATION ===
        void RegisterComponentSerializer(
            const std::string& componentName,
            ComponentSerializeFunc serializeFunc,
            ComponentDeserializeFunc deserializeFunc
        );

        void SerializeAllComponents(YAML::Emitter& emitter, EntityRegistry& registry, EntityID entity);

        void DeserializeAllComponents(
            const YAML::Node& node,
            EntityRegistry& registry,
            EntityID entity,
            AssetRegistry& assets
        );

        // === UTILITY ===
        bool IsAssetTypeRegistered(AssetType type) const;
        bool IsComponentTypeRegistered(const std::string& componentName) const;

        // Get list of registered types (useful for debugging/tooling)
        std::vector<AssetType> GetRegisteredAssetTypes() const;
        std::vector<std::string> GetRegisteredComponentNames() const;

    private:
        // Private constructor/destructor for singleton
        SerializationRegistry();
        ~SerializationRegistry();

        // Delete copy/move operations
        SerializationRegistry(const SerializationRegistry&) = delete;
        SerializationRegistry& operator=(const SerializationRegistry&) = delete;
        SerializationRegistry(SerializationRegistry&&) = delete;
        SerializationRegistry& operator=(SerializationRegistry&&) = delete;

        // PIMPL pattern to hide implementation details and maintain ABI stability
        // Using raw pointer instead of unique_ptr to avoid C4251 warning
        struct Impl;
        Impl* m_impl;
    };

    // === REGISTRATION HELPERS ===
    // These functions should be called during DLL initialization
    BOOM_API void RegisterAllAssetSerializers();
    BOOM_API void RegisterAllComponentSerializers();

    // Combined initialization - call this once on DLL load
    BOOM_API void InitializeSerializationSystem();
}