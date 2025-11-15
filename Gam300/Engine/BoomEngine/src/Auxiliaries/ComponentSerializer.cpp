#include "Core.h"
#include"BoomProperties.h"
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
                    e << YAML::Key << "CurrentClip" << YAML::Value << animatorComp.animator->GetCurrentClip();
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
                        size_t clipIndex = data["CurrentClip"].as<size_t>(0); // Default to clip 0
                        float time = data["Time"].as<float>(0.0f);

                        // Log what we're loading
                        BOOM_INFO("[AnimatorComponent] Prefab wants clip: {}, time: {}", clipIndex, time);
                        BOOM_INFO("[AnimatorComponent] Source animator has clip: {}",
                            skeletalModel->GetAnimator()->GetCurrentClip());

                        // Apply the saved state
                        newAnimator->PlayClip(clipIndex);
                        // Note: We removed SetTime() - you'll need to add it back to Animator

                        auto& animatorComp = reg.emplace<AnimatorComponent>(ent);
                        animatorComp.animator = newAnimator;

                    }
                }
            }
        );
        registry.RegisterComponentSerializer(
            "NavAgentComponent",
            // ----- SERIALIZE -----
            [](YAML::Emitter& e, EntityRegistry& reg, EntityID ent)
            {
                if (!reg.all_of<NavAgentComponent>(ent))
                    return;

                auto& nav = reg.get<NavAgentComponent>(ent);

                e << YAML::Key << "NavAgentComponent" << YAML::Value << YAML::BeginMap;

                // target
                e << YAML::Key << "Target" << YAML::Value
                    << YAML::Flow << YAML::BeginSeq
                    << nav.target.x << nav.target.y << nav.target.z
                    << YAML::EndSeq;

                e << YAML::Key << "Speed" << YAML::Value << nav.speed;
                e << YAML::Key << "ArriveRadius" << YAML::Value << nav.arrive;
                e << YAML::Key << "Active" << YAML::Value << nav.active;
                e << YAML::Key << "RepathCooldown" << YAML::Value << nav.repathCooldown;
                e << YAML::Key << "RetargetDistance" << YAML::Value << nav.retargetDist;

                e << YAML::EndMap;
            },
            // ----- DESERIALIZE -----
            [](const YAML::Node& data, EntityRegistry& reg, EntityID ent, AssetRegistry&)
            {
                if (!data || !data.IsMap())
                    return;

                auto& nav = reg.get_or_emplace<NavAgentComponent>(ent);

                // Target (vec3 as [x, y, z])
                if (auto t = data["Target"]; t && t.IsSequence() && t.size() == 3) {
                    nav.target.x = t[0].as<float>(nav.target.x);
                    nav.target.y = t[1].as<float>(nav.target.y);
                    nav.target.z = t[2].as<float>(nav.target.z);
                }

                if (auto v = data["Speed"])            nav.speed = v.as<float>(nav.speed);
                if (auto v = data["ArriveRadius"])     nav.arrive = v.as<float>(nav.arrive);
                if (auto v = data["Active"])           nav.active = v.as<bool>(nav.active);
                if (auto v = data["RepathCooldown"])   nav.repathCooldown = v.as<float>(nav.repathCooldown);
                if (auto v = data["RetargetDistance"]) nav.retargetDist = v.as<float>(nav.retargetDist);

                // Runtime-only stuff is intentionally NOT loaded:
                // nav.path, nav.waypoint, nav.dirty, nav.follow, nav.repathTimer
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

        // === THIRD PERSON CAMERA COMPONENT ===
        RegisterPropertyComponent<ThirdPersonCameraComponent>("ThirdPersonCameraComponent");

        
        RegisterPropertyComponent<AIComponent>("AIComponent");
       

        BOOM_INFO("[ComponentSerializers] All component serializers registered");
       
    }
 

   
}