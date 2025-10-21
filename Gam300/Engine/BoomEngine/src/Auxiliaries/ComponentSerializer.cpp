#include "Core.h"
#include "Auxiliaries\SerializationRegistry.h"
#include "ECS/ECS.hpp"

namespace Boom
{
    /**
     * @brief Generic component serializer using xproperty
     * Works for ANY component that has XPROPERTY_DEF defined
     */
    template<typename T>
    void RegisterPropertyComponent(const char* componentName)
    {
        auto& registry = SerializationRegistry::Instance();
        std::string name(componentName);

        registry.RegisterComponentSerializer(
            name,
            // ===== SERIALIZE =====
            [name](YAML::Emitter& e, EntityRegistry& scene, EntityID entity) {
                if (scene.all_of<T>(entity)) {
                    auto& comp = scene.get<T>(entity);

                    BOOM_INFO("[PropertySerializer] Serializing {} for entity {}", name, (uint32_t)entity);  // ADD THIS

                    e << YAML::Key << name << YAML::Value << YAML::BeginMap;

                    xproperty::settings::context ctx;
                    if (auto* pObj = xproperty::getObject(comp)) {
                        SerializeObjectToYAML(e, pObj, (void*)&comp, ctx);
                    }
                    else {
                        BOOM_ERROR("[PropertySerializer] Failed to get object info for {}", name);
                    }

                    e << YAML::EndMap;
                }
            },
            // ===== DESERIALIZE =====
            [name](const YAML::Node& node, EntityRegistry& scene, EntityID entity, AssetRegistry& /*assets*/) {
                BOOM_INFO("[PropertySerializer] Deserializing {} for entity {}", name, (uint32_t)entity);  // ADD THIS

                auto& comp = scene.get_or_emplace<T>(entity);

                if (node.IsMap()) {
                    xproperty::settings::context ctx;
                    if (auto* pObj = xproperty::getObject(comp)) {
                        DeserializeObjectFromYAML(node, pObj, (void*)&comp, ctx);
                        BOOM_INFO("[PropertySerializer] Successfully deserialized {}", name);  // ADD THIS
                    }
                    else {
                        BOOM_ERROR("[PropertySerializer] Failed to get object info for {} during deserialize", name);
                    }
                }
                else {
                    BOOM_WARN("[PropertySerializer] Node is not a map for {}", name);  // ADD THIS
                }
            }
        );
    }


    void RegisterAllComponentSerializers()
    {
        auto& registry = SerializationRegistry::Instance();

        // === INFO COMPONENT ===
        RegisterPropertyComponent<InfoComponent>("InfoComponent");

        // === TRANSFORM COMPONENT ===
        RegisterPropertyComponent<TransformComponent>("TransformComponent");

        // === CAMERA COMPONENT ===
        RegisterPropertyComponent<CameraComponent>("CameraComponent");

        // === RIGID BODY COMPONENT ===
		RegisterPropertyComponent<RigidBodyComponent>("RigidBodyComponent");

        // === COLLIDER COMPONENT ===
		RegisterPropertyComponent<ColliderComponent>("ColliderComponent");

        // === MODEL COMPONENT ===
		RegisterPropertyComponent<ModelComponent>("ModelComponent");

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
                        auto skeletalModel = std::dynamic_pointer_cast<SkeletalModel>(modelAsset.data);

                        // Clone the animator
                        auto newAnimator = skeletalModel->GetAnimator()->Clone();

                        // Get prefab's animation state
                        int32_t sequence = data["Sequence"].as<int32_t>();
                        float time = data["Time"].as<float>();

                        // Log what we're loading
                        BOOM_INFO("[AnimatorComponent] Prefab wants sequence: {}, time: {}", sequence, time);
                        BOOM_INFO("[AnimatorComponent] Source animator has sequence: {}",
                            skeletalModel->GetAnimator()->GetSequence());

                        newAnimator->SetSequence(sequence);
                        newAnimator->SetTime(time);

                        auto& animatorComp = reg.emplace<AnimatorComponent>(ent);
                        animatorComp.animator = newAnimator;
                    }
                }
            }
        );

        // === DIRECT LIGHT COMPONENT ===
		RegisterPropertyComponent<DirectLightComponent>("DirectLightComponent");

        // === POINT LIGHT COMPONENT ===
		RegisterPropertyComponent<PointLightComponent>("PointLightComponent");


        // === SPOT LIGHT COMPONENT ===
		RegisterPropertyComponent<SpotLightComponent>("SpotLightComponent");

        // === SKYBOX COMPONENT ===
		RegisterPropertyComponent<SkyboxComponent>("SkyboxComponent");

       

        BOOM_INFO("[ComponentSerializers] All component serializers registered");
    }
}