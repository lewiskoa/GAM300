#include "Core.h"
#include "../includes/Audio/SoundSystem.hpp"
#include "../includes/Audio/Audio.hpp"
#include "../includes/ECS/ECS.hpp"
#include <iostream>

using namespace Boom;

void SoundSystem::Shutdown()
{
	for (auto& [eid, name] : s_activeInstances) {
		SoundEngine::Instance().StopSound(name);
		SoundEngine::Instance().UnloadSound(name);
	}
	s_activeInstances.clear();
	s_lastPos.clear();
}

void SoundSystem::Update(Boom::EntityRegistry& registry, float dt)
{
	(void)dt; 
	auto view = registry.view<TransformComponent, SoundComponent>();

	for (auto entity : view)
	{
		auto& tf = view.get<TransformComponent>(entity);
		auto& sc = view.get<SoundComponent>(entity);

		uint64_t uid = static_cast<uint64_t>(static_cast<uint32_t>(entity));
		std::string instanceName = "ent_" + std::to_string(uid) + "_" + sc.name;

		if (sc.playOnStart && s_activeInstances.find(uid) == s_activeInstances.end())
		{
			SoundEngine::Instance().PreloadSound(instanceName, sc.filePath, false, sc.loop);
			SoundEngine::Instance().PlaySoundAt(instanceName, sc.filePath, tf.transform.translate, sc.loop);
			s_activeInstances[uid] = instanceName;
			s_lastPos[uid] = tf.transform.translate;
			continue;
		}

		auto it = s_activeInstances.find(uid);
		if (it != s_activeInstances.end())
		{
			const std::string& name = it->second;
			glm::vec3 pos = tf.transform.translate;
			SoundEngine::Instance().SetSoundPosition(name, pos);

			s_lastPos[uid] = pos;

			if (sc.filePath.empty())
			{
				SoundEngine::Instance().StopSound(name);
				SoundEngine::Instance().UnloadSound(name);
				s_activeInstances.erase(it);
				s_lastPos.erase(uid);
			}
		}
	}

	for (auto it = s_activeInstances.begin(); it != s_activeInstances.end(); )
	{
		auto eid = static_cast<Boom::EntityID>(static_cast<uint32_t>(it->first));
		if (!registry.valid(eid))
		{
			SoundEngine::Instance().StopSound(it->second);
			SoundEngine::Instance().UnloadSound(it->second);
			s_lastPos.erase(it->first);
			it = s_activeInstances.erase(it);
		}
		else ++it;
	}
}
