#pragma once
#define YAML_CPP_STATIC_DEFINE  // Add this for static linking
#include "Core.h"
#include "yaml-cpp/yaml.h"

namespace YAML
{
	//Change this to whatever data type we need to serialize/deserialize (SINCE GLM NOT ALLOWED OUTSIDE JRAFICS)
	//DONT YOU DARE SAY WE CANT USE GLM HERE AS WE ALSO NEED TO SERIALIZE GRAPHICS DATA RAAAAAAAAAAAAAAAH
	template<>
	struct convert<glm::vec3>
	{
		BOOM_INLINE static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}
		BOOM_INLINE static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template <typename Enum>
	BOOM_INLINE Enum deserializeEnum(const YAML::Node& node, Enum fallback) {
		if (!node) return fallback;

		// Try string form
		if (node.IsScalar()) {
			try {
				auto str = node.as<std::string>();
				if (auto e = magic_enum::enum_cast<Enum>(str))
					return *e;
			}
			catch (...) {
				// not a string, ignore
			}

			// Try integer form
			try {
				int val = node.as<int>();
				if (auto e = magic_enum::enum_cast<Enum>(val))
					return *e;
			}
			catch (...) {
				// not an int, ignore
			}
		}
		return fallback;
	}


	//stream operator
	BOOM_INLINE Emitter& operator<<(Emitter& out, const glm::vec3& v)
	{
		out << Flow;
		out << BeginSeq << v.x << v.y << v.z << EndSeq;
		return out;
	}
}