#include "Core.h"
#include "Auxiliaries\SerializationRegistry.h"
#include "ECS/ECS.hpp"

namespace Boom
{
    void RegisterAllComponentSerializers()
    {
        auto& registry = SerializationRegistry::Instance();

        // === INFO COMPONENT ===
        registry.RegisterComponentSerializer(
            "InfoComponent",
            // Serialize
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
            // Deserialize
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& info = reg.emplace<InfoComponent>(ent);
                info.uid = data["UID"].as<AssetID>();
                info.name = data["Name"].as<std::string>();
                info.parent = data["Parent"].as<AssetID>();
            }
        );

        // === TRANSFORM COMPONENT ===
        registry.RegisterComponentSerializer(
            "TransformComponent",
            // Serialize
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
            // Deserialize
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& t = reg.emplace<TransformComponent>(ent).transform;
                t.translate = data["Translate"].as<glm::vec3>();
                t.rotate = data["Rotate"].as<glm::vec3>();
                t.scale = data["Scale"].as<glm::vec3>();
            }
        );

        // === CAMERA COMPONENT ===
        registry.RegisterComponentSerializer(
            "CameraComponent",
            // Serialize
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
            // Deserialize
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& camera = reg.emplace<CameraComponent>(ent).camera;
                camera.nearPlane = data["NearPlane"].as<float>();
                camera.farPlane = data["FarPlane"].as<float>();
                camera.FOV = data["FOV"].as<float>();
            }
        );

        // === RIGID BODY COMPONENT ===
        registry.RegisterComponentSerializer(
            "RigidBodyComponent",
            // Serialize
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<RigidBodyComponent>(ent)) {
                    auto& rb = reg.get<RigidBodyComponent>(ent).RigidBody;
                    e << YAML::Key << "RigidBodyComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "Density" << YAML::Value << rb.density;
                    e << YAML::Key << "Type" << YAML::Value << std::string(magic_enum::enum_name(rb.type));
                    e << YAML::EndMap;
                }
            },
            // Deserialize
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& rb = reg.emplace<RigidBodyComponent>(ent).RigidBody;
                rb.density = data["Density"].as<float>();
                rb.type = YAML::deserializeEnum<RigidBody3D::Type>(data["Type"], RigidBody3D::DYNAMIC);
            }
        );

        // === COLLIDER COMPONENT ===
        registry.RegisterComponentSerializer(
            "ColliderComponent",
            // Serialize
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
            // Deserialize
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& col = reg.emplace<ColliderComponent>(ent).Collider;
                col.dynamicFriction = data["DynamicFriction"].as<float>();
                col.staticFriction = data["StaticFriction"].as<float>();
                col.restitution = data["Restitution"].as<float>();
                col.type = YAML::deserializeEnum<Collider3D::Type>(data["Type"], Collider3D::BOX);
            }
        );

        // === MODEL COMPONENT ===
        registry.RegisterComponentSerializer(
            "ModelComponent",
            // Serialize
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<ModelComponent>(ent)) {
                    auto& modelComp = reg.get<ModelComponent>(ent);
                    e << YAML::Key << "ModelComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "ModelID" << YAML::Value << modelComp.modelID;
                    e << YAML::Key << "MaterialID" << YAML::Value << modelComp.materialID;
                    e << YAML::EndMap;
                }
            },
            // Deserialize
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& comp = reg.emplace<ModelComponent>(ent);
                comp.materialID = data["MaterialID"].as<AssetID>();
                comp.modelID = data["ModelID"].as<AssetID>();
            }
        );

        // === ANIMATOR COMPONENT ===
        registry.RegisterComponentSerializer(
            "AnimatorComponent",
            // Serialize
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<AnimatorComponent>(ent)) {
                    auto& animatorComp = reg.get<AnimatorComponent>(ent);
                    e << YAML::Key << "AnimatorComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "Sequence" << YAML::Value << animatorComp.animator->GetSequence();
                    e << YAML::Key << "Time" << YAML::Value << animatorComp.animator->GetTime();
                    e << YAML::EndMap;
                }
            },
            // Deserialize
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry& assets) {
                if (reg.all_of<ModelComponent>(ent)) {
                    auto& modelComp = reg.get<ModelComponent>(ent);
                    ModelAsset& modelAsset = assets.Get<ModelAsset>(modelComp.modelID);

                    if (modelAsset.hasJoints) {
                        auto& animatorComp = reg.emplace<AnimatorComponent>(ent);
                        animatorComp.animator = std::dynamic_pointer_cast<SkeletalModel>(modelAsset.data)->GetAnimator();

                        int32_t sequence = data["Sequence"].as<int32_t>();
                        float time = data["Time"].as<float>();
                        animatorComp.animator->SetSequence(sequence);
                        animatorComp.animator->SetTime(time);
                    }
                }
            }
        );

        // === DIRECT LIGHT COMPONENT ===
        registry.RegisterComponentSerializer(
            "DirectLightComponent",
            // Serialize
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<DirectLightComponent>(ent)) {
                    auto& light = reg.get<DirectLightComponent>(ent).light;
                    e << YAML::Key << "DirectLightComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "Intensity" << YAML::Value << light.intensity;
                    e << YAML::Key << "Radiance" << YAML::Value << light.radiance;
                    e << YAML::EndMap;
                }
            },
            // Deserialize
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& light = reg.emplace<DirectLightComponent>(ent).light;
                light.intensity = data["Intensity"].as<float>();
                light.radiance = data["Radiance"].as<glm::vec3>();
            }
        );

        // === POINT LIGHT COMPONENT ===
        registry.RegisterComponentSerializer(
            "PointLightComponent",
            // Serialize
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<PointLightComponent>(ent)) {
                    auto& light = reg.get<PointLightComponent>(ent).light;
                    e << YAML::Key << "PointLightComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "Intensity" << YAML::Value << light.intensity;
                    e << YAML::Key << "Radiance" << YAML::Value << light.radiance;
                    e << YAML::EndMap;
                }
            },
            // Deserialize
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& light = reg.emplace<PointLightComponent>(ent).light;
                light.intensity = data["Intensity"].as<float>();
                light.radiance = data["Radiance"].as<glm::vec3>();
            }
        );

        // === SPOT LIGHT COMPONENT ===
        registry.RegisterComponentSerializer(
            "SpotLightComponent",
            // Serialize
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
            // Deserialize
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& light = reg.emplace<SpotLightComponent>(ent).light;
                light.intensity = data["Intensity"].as<float>();
                light.radiance = data["Radiance"].as<glm::vec3>();
                light.fallOff = data["Falloff"].as<float>();
                light.cutOff = data["Cutoff"].as<float>();
            }
        );

        // === SKYBOX COMPONENT ===
        registry.RegisterComponentSerializer(
            "SkyboxComponent",
            // Serialize
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent) {
                if (reg.all_of<SkyboxComponent>(ent)) {
                    auto& skybox = reg.get<SkyboxComponent>(ent).skyboxID;
                    e << YAML::Key << "SkyboxComponent" << YAML::Value << YAML::BeginMap;
                    e << YAML::Key << "SkyboxID" << YAML::Value << skybox;
                    e << YAML::EndMap;
                }
            },
            // Deserialize
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&) {
                auto& skybox = reg.emplace<SkyboxComponent>(ent).skyboxID;
                skybox = data["SkyboxID"].as<AssetID>();
            }
        );

       

        BOOM_INFO("[ComponentSerializers] All component serializers registered");
    }
}