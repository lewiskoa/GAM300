#pragma once
#include <tuple>
#include <string_view>
#include <glm/vec3.hpp>

namespace Boom {

	using EntityId = std::uint32_t;
	using Vec3 = glm::vec3;

	namespace CONSTANTS {
		constexpr int32_t STRING_BUFFER_SIZE{ 256 };
		constexpr std::tuple<float, float, float, float> DEFAULT_BACKGROUND_COLOR = {.3f, .3f, .3f, 1.f};
		constexpr std::string_view SHADERS_LOCATION = "Resources/Shaders/";
		constexpr std::string_view MODELS_LOCATION = "Resources/Models/";
		constexpr std::string_view TEXTURES_LOCATION = "Resources/Textures/";
		constexpr float CAM_PAN_SPEED{ .25f };
		constexpr float CAM_RUN_MULTIPLIER{ 1.5f }; 
		constexpr float MIN_FOV{ 45.f };
		constexpr float MAX_FOV{ 100.f };
		int const WINDOW_WIDTH = 1800;
		int const WINDOW_HEIGHT = 900;
		float const PI = 3.14159265358979323846f;
	}
}