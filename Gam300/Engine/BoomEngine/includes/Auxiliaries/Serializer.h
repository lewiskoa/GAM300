#pragma once
#include "ECS/ECS.hpp"
#include "Common/YAML.h"

namespace Boom
{
	struct DataSerializer
	{
		BOOM_INLINE void Serialize(EntityRegistry& scene, const std::string& path)
		{
			//serialize scene
			YAML::Emitter emitter;
			emitter << YAML::BeginMap;
			{
				emitter << YAML::Key << "ENTITIES" << YAML::Value << YAML::BeginSeq;
				{
					scene.each([&](EntityID entt)
						{
							Entity entity{ &scene, entt };
							emitter << YAML::BeginMap;
							{
								if (entity.Has<InfoComponent>())
								{
									auto& info = entity.Get<InfoComponent>();
									emitter << YAML::Key << "InfoComponent" << YAML::Value << YAML::BeginMap;
									{
										emitter << YAML::Key << "UID" << YAML::Value << info.uid;
										emitter << YAML::Key << "Name" << YAML::Value << info.name;
										emitter << YAML::Key << "Parent" << YAML::Value << info.parent;
									}
									emitter << YAML::EndMap;
								}

								if (entity.Has<CameraComponent>())
								{
									auto& camera = entity.Get<CameraComponent>().camera;
									emitter << YAML::Key << "CameraComponent" << YAML::Value << YAML::BeginMap;
									{
										emitter << YAML::Key << "NearPlane" << YAML::Value << camera.nearPlane;
										emitter << YAML::Key << "FarPlane" << YAML::Value << camera.farPlane;
										emitter << YAML::Key << "FOV" << YAML::Value << camera.FOV;
									}
									emitter << YAML::EndMap;
								}

								if (entity.Has<TransformComponent>())
								{
									auto& transform = entity.Get<TransformComponent>().transform;
									emitter << YAML::Key << "TransformComponent" << YAML::Value << YAML::BeginMap;
									{
										emitter << YAML::Key << "Translate" << YAML::Value << transform.translate;
										emitter << YAML::Key << "Rotate" << YAML::Value << transform.rotate;
										emitter << YAML::Key << "Scale" << YAML::Value << transform.scale;
									}
									emitter << YAML::EndMap;
								}

								//serialize rigidbody
								if(entity.Has<RigidBodyComponent>())
								{
									auto& rb = entity.Get<RigidBodyComponent>().RigidBody;
									emitter << YAML::Key << "RigidBodyComponent" << YAML::Value << YAML::BeginMap;
									{
										emitter << YAML::Key << "Density" << YAML::Value << rb.density;
										emitter << YAML::Key << "Type" << YAML::Value << rb.type;
									}
									emitter << YAML::EndMap;
								}
								
								//serialize collider
								if(entity.Has<ColliderComponent>())
								{
									auto& col = entity.Get<ColliderComponent>().Collider;
									emitter << YAML::Key << "ColliderComponent" << YAML::Value << YAML::BeginMap;
									{
										emitter << YAML::Key << "DynamicFriction" << YAML::Value << col.dynamicFriction;
										emitter << YAML::Key << "StaticFriction" << YAML::Value << col.staticFriction;
										emitter << YAML::Key << "Restitution" << YAML::Value << col.restitution;
										emitter << YAML::Key << "Type" << YAML::Value << col.type;
									}
									emitter << YAML::EndMap;
								}

								//serialize Directlight
								if (entity.Has<DirectLightComponent>())
								{
									auto& light = entity.Get<DirectLightComponent>().light;
									emitter << YAML::Key << "DirectLightComponent" << YAML::Value << YAML::BeginMap;
									{
										emitter << YAML::Key << "Intensity" << YAML::Value << light.intensity;
										emitter << YAML::Key << "Radiance" << YAML::Value << light.radiance;
										//emitter << YAML::Key << "Bias" << YAML::Value << light.shadowBias;
									}
									emitter << YAML::EndMap;
								}

								//serialize pointlight
								if(entity.Has<PointLightComponent>())
								{
									auto& light = entity.Get<PointLightComponent>().light;
									emitter << YAML::Key << "PointLightComponent" << YAML::Value << YAML::BeginMap;
									{
										emitter << YAML::Key << "Intensity" << YAML::Value << light.intensity;
										emitter << YAML::Key << "Radiance" << YAML::Value << light.radiance;
									}
									emitter << YAML::EndMap;
								}

								//serialize spotlight
								if(entity.Has<SpotLightComponent>())
								{
									auto& light = entity.Get<SpotLightComponent>().light;
									emitter << YAML::Key << "SpotLightComponent" << YAML::Value << YAML::BeginMap;
									{
										emitter << YAML::Key << "Intensity" << YAML::Value << light.intensity;
										emitter << YAML::Key << "Radiance" << YAML::Value << light.radiance;
										emitter << YAML::Key << "Falloff" << YAML::Value << light.fallOff;
										emitter << YAML::Key << "Cutoff" << YAML::Value << light.cutOff;
										
									}
									emitter << YAML::EndMap;
								}

								if (entity.Has<SkyboxComponent>())
								{
									auto& skybox = entity.Get<SkyboxComponent>().skyboxID;
									emitter << YAML::Key << "SkyboxComponent" << YAML::Value << YAML::BeginMap;
									{

										emitter << YAML::Key << "SkyboxID" << YAML::Value << skybox;
									}
									emitter << YAML::EndMap;
								}

								if(entity.Has<ModelComponent>())
								{
									auto& modelComp = entity.Get<ModelComponent>();
									emitter << YAML::Key << "ModelComponent" << YAML::Value << YAML::BeginMap;
									{
										emitter << YAML::Key << "ModelID" << YAML::Value << modelComp.modelID;
										emitter << YAML::Key << "MaterialID" << YAML::Value << modelComp.materialID;
									}
									emitter << YAML::EndMap;
								}
							}
							emitter << YAML::EndMap;
						});
				}
				emitter << YAML::EndSeq;
			}
			emitter << YAML::EndMap;
			std::ofstream filepath(path);
			filepath << emitter.c_str();
		}

		BOOM_INLINE void Serialize(AssetRegistry& registry, const std::string& path)
		{

		}


		BOOM_INLINE void Deserialize(EntityRegistry& scene, const std::string& path)
		{
			//deserialize scene
			try
			{
				auto root = YAML::LoadFile(path);
				const auto& nodes = root["ENTITIES"];
				scene.clear();

				for (auto& node : nodes)
				{
					EntityID entity = scene.create();

					//deserialize entt infos
					if(auto& data = node["InfoComponent"])
					{
						auto& info = scene.emplace<InfoComponent>(entity);
						info.uid = data["UID"].as<AssetID>();
						info.name = data["Name"].as<std::string>();
						info.parent = data["Parent"].as<AssetID>();
					}

					//deserialize Camera
					if(auto& data = node["CameraComponent"])
					{
						auto& camera = scene.emplace<CameraComponent>(entity).camera;
						camera.nearPlane = data["NearPlane"].as<float>();
						camera.farPlane = data["FarPlane"].as<float>();
						camera.FOV = data["FOV"].as<float>();
					}

					//deserialize Transform
					if(auto& data = node["TransformComponent"])
					{
						auto& transform = scene.emplace<TransformComponent>(entity).transform;
						transform.translate = data["Translate"].as<glm::vec3>();
						transform.rotate = data["Rotate"].as<glm::vec3>();
						transform.scale = data["Scale"].as<glm::vec3>();
					}

					//deserialize rigidbody
					if(auto& data = node["RigidBodyComponent"])
					{
						auto& rb = scene.emplace<RigidBodyComponent>(entity).RigidBody;
						rb.density = data["Density"].as<float>();
						rb.type = data["Type"].as<>();
					}
				}

			}
			catch(YAML::ParserException& e)
			{
				BOOM_ERROR("Failed to deserialize scene!");
			}
		}

		BOOM_INLINE void Deserialize(AssetRegistry& registry, const std::string& path)
		{
			//deserialize assets
		}
	};
}