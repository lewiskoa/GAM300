#include "Core.h"
#include "Auxiliaries\SerializationRegistry.h"
#include <unordered_map>
#include <iostream>

namespace Boom
{
    // === PIMPL IMPLEMENTATION ===
    struct SerializationRegistry::Impl
    {
        struct AssetEntry
        {
            AssetSerializePropsFunc serializeProps;
            AssetDeserializeFunc deserialize;
        };

        struct ComponentEntry
        {
            ComponentSerializeFunc serialize;
            ComponentDeserializeFunc deserialize;
        };

        std::unordered_map<AssetType, AssetEntry> assetRegistry;
        std::unordered_map<std::string, ComponentEntry> componentRegistry;
    };

    // === SINGLETON IMPLEMENTATION ===
    SerializationRegistry& SerializationRegistry::Instance()
    {
        // This static variable lives in the DLL's data segment
        static SerializationRegistry instance;
        return instance;
    }

    SerializationRegistry::SerializationRegistry()
        : m_impl(new Impl())
    {
        BOOM_INFO("[SerializationRegistry] Initialized");
    }

    SerializationRegistry::~SerializationRegistry()
    {
        delete m_impl;
        BOOM_INFO("[SerializationRegistry] Destroyed");
    }

    // === ASSET SERIALIZATION ===
    void SerializationRegistry::RegisterAssetSerializer(
        AssetType type,
        AssetSerializePropsFunc serializeFunc,
        AssetDeserializeFunc deserializeFunc)
    {
        m_impl->assetRegistry[type] = { serializeFunc, deserializeFunc };
    }

    void SerializationRegistry::SerializeAssetProperties(YAML::Emitter& emitter, Asset* asset)
    {
        auto it = m_impl->assetRegistry.find(asset->type);
        if (it != m_impl->assetRegistry.end() && it->second.serializeProps)
        {
            it->second.serializeProps(emitter, asset);
        }
        else
        {
            std::cout << "[SerializationRegistry] No serializer registered for asset type: "
				<< magic_enum::enum_name(asset->type) << std::endl;
        }
    }

    Asset* SerializationRegistry::DeserializeAsset(
        AssetRegistry& registry,
        AssetType type,
        AssetID uid,
        const std::string& source,
        const YAML::Node& properties)
    {
        auto it = m_impl->assetRegistry.find(type);
        if (it != m_impl->assetRegistry.end() && it->second.deserialize)
        {
            return it->second.deserialize(registry, uid, source, properties);
        }

		std::cout << "[SerializationRegistry] No deserializer registered for asset type: " << magic_enum::enum_name(type) << std::endl;
        return nullptr;
    }

    // === COMPONENT SERIALIZATION ===
    void SerializationRegistry::RegisterComponentSerializer(
        const std::string& componentName,
        ComponentSerializeFunc serializeFunc,
        ComponentDeserializeFunc deserializeFunc)
    {
        m_impl->componentRegistry[componentName] = { serializeFunc, deserializeFunc };
        BOOM_INFO("[SerializationRegistry] Registered component serializer: {}", componentName);
    }

    void SerializationRegistry::SerializeAllComponents(
        YAML::Emitter& emitter,
        EntityRegistry& registry,
        EntityID entity)
    {
        for (const auto& [name, entry] : m_impl->componentRegistry)
        {
            if (entry.serialize)
            {
                entry.serialize(emitter, registry, entity);
            }
        }
    }

    void SerializationRegistry::DeserializeAllComponents(
        const YAML::Node& node,
        EntityRegistry& registry,
        EntityID entity,
        AssetRegistry& assets)
    {
        for (const auto& [name, entry] : m_impl->componentRegistry)
        {
            if (node[name] && entry.deserialize)
            {
                entry.deserialize(node[name], registry, entity, assets);
            }
        }
    }

    // === UTILITY FUNCTIONS ===
    bool SerializationRegistry::IsAssetTypeRegistered(AssetType type) const
    {
        return m_impl->assetRegistry.find(type) != m_impl->assetRegistry.end();
    }

    bool SerializationRegistry::IsComponentTypeRegistered(const std::string& componentName) const
    {
        return m_impl->componentRegistry.find(componentName) != m_impl->componentRegistry.end();
    }

    std::vector<AssetType> SerializationRegistry::GetRegisteredAssetTypes() const
    {
        std::vector<AssetType> types;
        types.reserve(m_impl->assetRegistry.size());
        for (const auto& [type, _] : m_impl->assetRegistry)
        {
            types.push_back(type);
        }
        return types;
    }

    std::vector<std::string> SerializationRegistry::GetRegisteredComponentNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_impl->componentRegistry.size());
        for (const auto& [name, _] : m_impl->componentRegistry)
        {
            names.push_back(name);
        }
        return names;
    }
}