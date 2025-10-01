#pragma once
#include "ECS/ECS.hpp"
#include "Common/YAML.h"
#include <functional>
#include <unordered_map>

namespace Boom
{
    class ComponentSerializer
    {
    public:
        using SerializeFunc = std::function<void(YAML::Emitter&, EntityRegistry&, EntityID)>;
        using DeserializeFunc = std::function<void(const YAML::Node&, EntityRegistry&, EntityID, AssetRegistry&)>;

        BOOM_INLINE static void Register(const std::string& name, SerializeFunc ser, DeserializeFunc deser)
        {
            GetRegistry()[name] = { ser, deser };
        }

        BOOM_INLINE static void SerializeAll(YAML::Emitter& e, EntityRegistry& reg, EntityID ent)
        {
            for (auto& [name, funcs] : GetRegistry())
            {
                funcs.serialize(e, reg, ent);
            }
        }

        BOOM_INLINE static void DeserializeAll(const YAML::Node& node, EntityRegistry& reg, EntityID ent, AssetRegistry& assets)
        {
            for (auto& [name, funcs] : GetRegistry())
            {
                if (node[name])
                {
                    funcs.deserialize(node[name], reg, ent, assets);
                }
            }
        }

    private:
        struct Entry { SerializeFunc serialize; DeserializeFunc deserialize; };

        BOOM_INLINE static std::unordered_map<std::string, Entry>& GetRegistry()
        {
            static std::unordered_map<std::string, Entry> registry;
            return registry;
        }
    };

    // Call this once at startup
    BOOM_INLINE void RegisterAllComponents()
    {
        // InfoComponent
        ComponentSerializer::Register("InfoComponent",
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<InfoComponent>(ent)) {
                    auto& info = reg.get<InfoComponent>(ent);
                    e << YAML::Key << "InfoComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "UID" << YAML::Value << info.uid;
                    e << YAML::Key << "Name" << YAML::Value << info.name;
                    e << YAML::Key << "Parent" << YAML::Value << info.parent;
                    e << YAML::EndMap;
                }
            },
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& info = reg.emplace<InfoComponent>(ent);
                info.uid = data["UID"].as<AssetID>();
                info.name = data["Name"].as<std::string>();
                info.parent = data["Parent"].as<AssetID>();
            });

        // TransformComponent
        ComponentSerializer::Register("TransformComponent",
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<TransformComponent>(ent)) {
                    auto& t = reg.get<TransformComponent>(ent).transform;
                    e << YAML::Key << "TransformComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "Translate" << YAML::Value << t.translate;
                    e << YAML::Key << "Rotate" << YAML::Value << t.rotate;
                    e << YAML::Key << "Scale" << YAML::Value << t.scale;
                    e << YAML::EndMap;
                }
            },
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& t = reg.emplace<TransformComponent>(ent).transform;
                t.translate = data["Translate"].as<glm::vec3>();
                t.rotate = data["Rotate"].as<glm::vec3>();
                t.scale = data["Scale"].as<glm::vec3>();
            });

        // CameraComponent
        ComponentSerializer::Register("CameraComponent",
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<CameraComponent>(ent)) {
                    auto& camera = reg.get<CameraComponent>(ent).camera;
                    e << YAML::Key << "CameraComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "NearPlane" << YAML::Value << camera.nearPlane;
                    e << YAML::Key << "FarPlane" << YAML::Value << camera.farPlane;
                    e << YAML::Key << "FOV" << YAML::Value << camera.FOV;
                    e << YAML::EndMap;
                }
            },
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& camera = reg.emplace<CameraComponent>(ent).camera;
                camera.nearPlane = data["NearPlane"].as<float>();
                camera.farPlane = data["FarPlane"].as<float>();
                camera.FOV = data["FOV"].as<float>();
            });

        // RigidBodyComponent
        ComponentSerializer::Register("RigidBodyComponent",
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<RigidBodyComponent>(ent)) {
                    auto& rb = reg.get<RigidBodyComponent>(ent).RigidBody;
                    e << YAML::Key << "RigidBodyComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "Density" << YAML::Value << rb.density;
                    e << YAML::Key << "Type" << YAML::Value << std::string(magic_enum::enum_name(rb.type));
                    e << YAML::EndMap;
                }
            },
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& rb = reg.emplace<RigidBodyComponent>(ent).RigidBody;
                rb.density = data["Density"].as<float>();
                rb.type = YAML::deserializeEnum<RigidBody3D::Type>(data["Type"], RigidBody3D::DYNAMIC);
            });

        // ColliderComponent
        ComponentSerializer::Register("ColliderComponent",
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<ColliderComponent>(ent)) {
                    auto& col = reg.get<ColliderComponent>(ent).Collider;
                    e << YAML::Key << "ColliderComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "DynamicFriction" << YAML::Value << col.dynamicFriction;
                    e << YAML::Key << "StaticFriction" << YAML::Value << col.staticFriction;
                    e << YAML::Key << "Restitution" << YAML::Value << col.restitution;
                    e << YAML::Key << "Type" << YAML::Value << std::string(magic_enum::enum_name(col.type));
                    e << YAML::EndMap;
                }
            },
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& col = reg.emplace<ColliderComponent>(ent).Collider;
                col.dynamicFriction = data["DynamicFriction"].as<float>();
                col.staticFriction = data["StaticFriction"].as<float>();
                col.restitution = data["Restitution"].as<float>();
                col.type = YAML::deserializeEnum<Collider3D::Type>(data["Type"], Collider3D::BOX);
            });

        // ModelComponent
        ComponentSerializer::Register("ModelComponent",
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<ModelComponent>(ent)) {
                    auto& modelComp = reg.get<ModelComponent>(ent);
                    e << YAML::Key << "ModelComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "ModelID" << YAML::Value << modelComp.modelID;
                    e << YAML::Key << "MaterialID" << YAML::Value << modelComp.materialID;
                    e << YAML::EndMap;
                }
            },
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& comp = reg.emplace<ModelComponent>(ent);
                comp.materialID = data["MaterialID"].as<AssetID>();
                comp.modelID = data["ModelID"].as<AssetID>();
            });

        // AnimatorComponent
        ComponentSerializer::Register("AnimatorComponent",
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<AnimatorComponent>(ent)) {
                    auto& animatorComp = reg.get<AnimatorComponent>(ent);
                    e << YAML::Key << "AnimatorComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "Sequence" << YAML::Value << animatorComp.animator->GetSequence();
                    e << YAML::Key << "Time" << YAML::Value << animatorComp.animator->GetTime();
                    e << YAML::EndMap;
                }
            },
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry& assets) {
                // Only add animator if the entity has a model component
                if (reg.all_of<ModelComponent>(ent)) {
                    auto& modelComp = reg.get<ModelComponent>(ent);
                    ModelAsset& modelAsset = assets.Get<ModelAsset>(modelComp.modelID);

                    // Only skeletal models have animators
                    if (modelAsset.hasJoints) {
                        auto& animatorComp = reg.emplace<AnimatorComponent>(ent);
                        animatorComp.animator = std::dynamic_pointer_cast<SkeletalModel>(modelAsset.data)->GetAnimator();

                        // Restore animation state
                        int32_t sequence = data["Sequence"].as<int32_t>();
                        float time = data["Time"].as<float>();
                        animatorComp.animator->SetSequence(sequence);
                        animatorComp.animator->SetTime(time);
                    }
                }
            });

        // DirectLightComponent
        ComponentSerializer::Register("DirectLightComponent",
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<DirectLightComponent>(ent)) {
                    auto& light = reg.get<DirectLightComponent>(ent).light;
                    e << YAML::Key << "DirectLightComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "Intensity" << YAML::Value << light.intensity;
                    e << YAML::Key << "Radiance" << YAML::Value << light.radiance;
                    e << YAML::EndMap;
                }
            },
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& light = reg.emplace<DirectLightComponent>(ent).light;
                light.intensity = data["Intensity"].as<float>();
                light.radiance = data["Radiance"].as<glm::vec3>();
            });

        // PointLightComponent
        ComponentSerializer::Register("PointLightComponent",
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<PointLightComponent>(ent)) {
                    auto& light = reg.get<PointLightComponent>(ent).light;
                    e << YAML::Key << "PointLightComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "Intensity" << YAML::Value << light.intensity;
                    e << YAML::Key << "Radiance" << YAML::Value << light.radiance;
                    e << YAML::EndMap;
                }
            },
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& light = reg.emplace<PointLightComponent>(ent).light;
                light.intensity = data["Intensity"].as<float>();
                light.radiance = data["Radiance"].as<glm::vec3>();
            });

        // SpotLightComponent
        ComponentSerializer::Register("SpotLightComponent",
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<SpotLightComponent>(ent)) {
                    auto& light = reg.get<SpotLightComponent>(ent).light;
                    e << YAML::Key << "SpotLightComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "Intensity" << YAML::Value << light.intensity;
                    e << YAML::Key << "Radiance" << YAML::Value << light.radiance;
                    e << YAML::Key << "Falloff" << YAML::Value << light.fallOff;
                    e << YAML::Key << "Cutoff" << YAML::Value << light.cutOff;
                    e << YAML::EndMap;
                }
            },
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& light = reg.emplace<SpotLightComponent>(ent).light;
                light.intensity = data["Intensity"].as<float>();
                light.radiance = data["Radiance"].as<glm::vec3>();
                light.fallOff = data["Falloff"].as<float>();
                light.cutOff = data["Cutoff"].as<float>();
            });

        // SkyboxComponent
        ComponentSerializer::Register("SkyboxComponent",
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<SkyboxComponent>(ent)) {
                    auto& skybox = reg.get<SkyboxComponent>(ent).skyboxID;
                    e << YAML::Key << "SkyboxComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "SkyboxID" << YAML::Value << skybox;
                    e << YAML::EndMap;
                }
            },
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& skybox = reg.emplace<SkyboxComponent>(ent).skyboxID;
                skybox = data["SkyboxID"].as<AssetID>();
            });

        // SoundComponent
        ComponentSerializer::Register("SoundComponent",
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<SoundComponent>(ent)) {
                    auto& sound = reg.get<SoundComponent>(ent);
                    e << YAML::Key << "SoundComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "Name" << YAML::Value << sound.name;
                    e << YAML::Key << "FilePath" << YAML::Value << sound.filePath;
                    e << YAML::Key << "Loop" << YAML::Value << sound.loop;
                    e << YAML::Key << "Volume" << YAML::Value << sound.volume;
                    e << YAML::Key << "PlayOnStart" << YAML::Value << sound.playOnStart;
                    e << YAML::EndMap;
                }
            },
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& sound = reg.emplace<SoundComponent>(ent);
                sound.name = data["Name"].as<std::string>();
                sound.filePath = data["FilePath"].as<std::string>();
                sound.loop = data["Loop"].as<bool>();
                sound.volume = data["Volume"].as<float>();
                sound.playOnStart = data["PlayOnStart"].as<bool>();
            });
    }
}