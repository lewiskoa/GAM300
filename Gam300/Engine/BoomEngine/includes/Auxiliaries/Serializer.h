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
					// Replace the deprecated .each() call with .view()
					auto view = scene.view<entt::entity>();
					for (auto entt : view)
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
							if (entity.Has<RigidBodyComponent>())
							{
								auto& rb = entity.Get<RigidBodyComponent>().RigidBody;
								emitter << YAML::Key << "RigidBodyComponent" << YAML::Value << YAML::BeginMap;
								{
									emitter << YAML::Key << "Density" << YAML::Value << rb.density;
									emitter << YAML::Key << "Type" << YAML::Value << std::string(magic_enum::enum_name(rb.type));
								}
								emitter << YAML::EndMap;
							}

							//serialize collider
							if (entity.Has<ColliderComponent>())
							{
								auto& col = entity.Get<ColliderComponent>().Collider;
								emitter << YAML::Key << "ColliderComponent" << YAML::Value << YAML::BeginMap;
								{
									emitter << YAML::Key << "DynamicFriction" << YAML::Value << col.dynamicFriction;
									emitter << YAML::Key << "StaticFriction" << YAML::Value << col.staticFriction;
									emitter << YAML::Key << "Restitution" << YAML::Value << col.restitution;
									emitter << YAML::Key << "Type" << YAML::Value << std::string(magic_enum::enum_name(col.type));
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
							if (entity.Has<PointLightComponent>())
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
							if (entity.Has<SpotLightComponent>())
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

							if (entity.Has<ModelComponent>())
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
					}
				}
				emitter << YAML::EndSeq;
			}
			emitter << YAML::EndMap;
			std::ofstream filepath(path);
			filepath << emitter.c_str();
		}

		BOOM_INLINE void Serialize(AssetRegistry& registry, const std::string& path)
		{
			YAML::Emitter emitter;

			emitter << YAML::BeginMap;
			{
				emitter << YAML::Key << "ASSETS" << YAML::Value << YAML::BeginSeq;
				{
					registry.View([&](Asset* asset)
					{
						//asset type to string
						std::string typeName(magic_enum::enum_name(asset->type));
						emitter << YAML::BeginMap;
						{
							emitter << YAML::Key << "Type" << YAML::Value << typeName;
							emitter << YAML::Key << "UID" << YAML::Value << asset->uid;
							emitter << YAML::Key << "Name" << YAML::Value << asset->name;
							emitter << YAML::Key << "Source" << YAML::Value << asset->source;

							if(asset->type == AssetType::MATERIAL)
							{
								auto mtl = static_cast<MaterialAsset*>(asset);
								emitter << "Properties" << YAML::BeginMap;
								{
									//texture assets
									emitter << YAML::Key << "AlbedoMap"		<< YAML::Value << mtl->albedoMapID;
									emitter << YAML::Key << "NormalMap"		<< YAML::Value << mtl->normalMapID;
									emitter << YAML::Key << "RoughnessMap"	<< YAML::Value << mtl->roughnessMapID;
									emitter << YAML::Key << "MetallicMap"	<< YAML::Value << mtl->metallicMapID;
									emitter << YAML::Key << "OcclusionMap"	<< YAML::Value << mtl->occlusionMapID;
									emitter << YAML::Key << "EmissiveMap"	<< YAML::Value << mtl->emissiveMapID;

									//properties
									emitter << YAML::Key << "Albedo"	<< YAML::Value << mtl->data.albedo;
									emitter << YAML::Key << "Metallic"	<< YAML::Value << mtl->data.metallic;
									emitter << YAML::Key << "Roughness" << YAML::Value << mtl->data.roughness;
									emitter << YAML::Key << "Occlusion" << YAML::Value << mtl->data.occlusion;
									emitter << YAML::Key << "Emissive"	<< YAML::Value << mtl->data.emissive;
								}
								emitter << YAML::EndMap;
							}
							else if (asset->type == AssetType::TEXTURE)
							{
								auto tex = static_cast<TextureAsset*>(asset);
								emitter << "Properties" << YAML::BeginMap;
								{
									emitter << YAML::Key << "IsHDR"		<<	YAML::Value << tex->isHDR;
									emitter << YAML::Key << "IsFlipY"	<<	YAML::Value << tex->isFlipY;
								}
								emitter << YAML::EndMap;
							}
							else if (asset->type == AssetType::SKYBOX)
							{
								auto skybox = static_cast<SkyboxAsset*>(asset);
								emitter << "Properties" << YAML::BeginMap;
								{
									emitter << YAML::Key << "Size"		<<	YAML::Value << skybox->size;
									emitter << YAML::Key << "IsHDR"		<<	YAML::Value << skybox->isHDR;
									emitter << YAML::Key << "IsFlipY"	<<	YAML::Value << skybox->isFlipY;
								}
								emitter << YAML::EndMap;
							}
							else if (asset->type == AssetType::MODEL)
							{
								auto model = static_cast<ModelAsset*>(asset);
								emitter << "Properties" << YAML::BeginMap;
								{
									emitter << YAML::Key << "HasJoints" << YAML::Value << model->hasJoints;
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
						rb.type = YAML::deserializeEnum<RigidBody3D::Type>(data["Type"], RigidBody3D::DYNAMIC);
					}

					//deserialize collider
					if(auto& data = node["ColliderComponent"])
					{
						auto& col = scene.emplace<ColliderComponent>(entity).Collider;
						col.dynamicFriction = data["DynamicFriction"].as<float>();
						col.staticFriction = data["StaticFriction"].as<float>();
						col.restitution = data["Restitution"].as<float>();
						col.type = YAML::deserializeEnum<Collider3D::Type>(data["Type"], Collider3D::BOX);
					}

					//deserialize model
					if (auto& data = node["ModelComponent"])
					{
						auto& comp = scene.emplace<ModelComponent>(entity);
						comp.materialID = data["MaterialID"].as<AssetID>();
						comp.modelID = data["ModelID"].as<AssetID>();
					}

					//deserialize directlight
					if(auto& data = node["DirectLightComponent"])
					{
						auto& light = scene.emplace<DirectLightComponent>(entity).light;
						light.intensity = data["Intensity"].as<float>();
						light.radiance = data["Radiance"].as<glm::vec3>();
						//light.shadowBias = data["Bias"].as<float>();
					}

					//deserialize pointlight
					if(auto& data = node["PointLightComponent"])
					{
						auto& light = scene.emplace<PointLightComponent>(entity).light;
						light.intensity = data["Intensity"].as<float>();
						light.radiance = data["Radiance"].as<glm::vec3>();
					}

					//deserialize spotlight
					if(auto& data = node["SpotLightComponent"])
					{
						auto& light = scene.emplace<SpotLightComponent>(entity).light;
						light.intensity = data["Intensity"].as<float>();
						light.radiance = data["Radiance"].as<glm::vec3>();
						light.fallOff = data["Falloff"].as<float>();
						light.cutOff = data["Cutoff"].as<float>();
					}
					//deserialize skybox
					if (auto& data = node["SkyboxComponent"])
					{
						auto& skybox = scene.emplace<SkyboxComponent>(entity).skyboxID;
						skybox = data["SkyboxID"].as<AssetID>();
					}
				}

			}
			catch(YAML::ParserException& e)
			{
				BOOM_ERROR("Failed to deserialize scene!", e.what());
			}
		}

		BOOM_INLINE void Deserialize(AssetRegistry& registry, const std::string& path)
		{
			//deserialize assets
			try
			{
				auto root = YAML::LoadFile(path);
				const auto& nodes = root["ASSETS"];

				for (auto& node : nodes)
				{
					Asset* asset	=	nullptr;
					auto props		=	node["Properties"];
					auto uid		=	node["UID"].as<AssetID>();
					auto name		=	node["Name"].as<std::string>();
					auto source		=	node["Source"].as<std::string>();

					//get asset type
					auto typeName	=	node["Type"].as<std::string>();
					auto opt		=	magic_enum::enum_cast<AssetType>(typeName);
					auto type		=	(opt.has_value()) ? opt.value() : AssetType::UNKNOWN;

					BOOM_INFO("[Deserialize] Processing asset UID={}, Type={}", uid, typeName);

					//create instance
					if (type == AssetType::MATERIAL && props)
					{
						auto mtlAssest = registry.AddMaterial(uid, source);

						//set material textures
						mtlAssest->albedoMapID		=	props["AlbedoMap"].as<AssetID>();
						mtlAssest->normalMapID		=	props["NormalMap"].as<AssetID>();
						mtlAssest->roughnessMapID	=	props["RoughnessMap"].as<AssetID>();
						mtlAssest->metallicMapID	=	props["MetallicMap"].as<AssetID>();
						mtlAssest->occlusionMapID	=	props["OcclusionMap"].as<AssetID>();
						mtlAssest->emissiveMapID	=	props["EmissiveMap"].as<AssetID>();

						//SET MATERIAL PROPERTIES
						mtlAssest->data.albedo		=	props["Albedo"].as<glm::vec3>();
						mtlAssest->data.roughness	=	props["Roughness"].as<float>();
						mtlAssest->data.metallic	=	props["Metallic"].as<float>();
						mtlAssest->data.occlusion	=	props["Occlusion"].as<float>();
						mtlAssest->data.emissive	=	props["Emissive"].as<glm::vec3>();

						//cast asset
						asset = static_cast<Asset*>(mtlAssest.get());
					}
					else if (type == AssetType::TEXTURE && props)
					{
						bool isHDR	 = props["IsHDR"].as<bool>();
						bool isFlipY = props["IsFlipY"].as<bool>();
						asset = (Asset*)registry.AddTexture(uid, source, isHDR, isFlipY).get();
					}
					else if (type == AssetType::SKYBOX && props)
					{
						int32_t size	= props["Size"].as<int32_t>();
						bool isHDR		= props["IsHDR"].as<bool>();
						bool isFlipY	= props["IsFlipY"].as<bool>();
						asset = (Asset*)registry.AddSkybox(uid, source, size, isHDR, isFlipY).get();
					}
					else if (type == AssetType::MODEL && props)
					{
						bool hasJoints = props["HasJoints"].as<bool>();
						asset = (Asset*)registry.AddModel(uid, source, hasJoints).get();
					}
					/*else if (type == AssetType::SCRIPT)
					{
						asset = (Asset*)registry.AddScript(uid, source).get();
					}*/
					else if (type == AssetType::SCENE)
					{
						asset = (Asset*)registry.AddScene(uid, source).get();
					}
					else
					{
						BOOM_ERROR("Failed to deserialize asset: invalid type!");
						continue;
					}

					//set common properties
					asset->source = source;
					asset->name = name;
				}
			}
			catch (YAML::ParserException& e)
			{
				BOOM_ERROR("Failed to deserialize assets!", e.what());
			}
		}
	};
}