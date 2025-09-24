#pragma once
#include <tuple>
#include <string_view>

namespace Boom {
	namespace CONSTANTS {
		constexpr std::tuple<float, float, float, float> DEFAULT_BACKGROUND_COLOR = {.3f, .3f, .3f, 1.f};
		constexpr std::string_view RESOURCES_LOCATION = "Resources";
		constexpr std::string_view SHADERS_LOCATION = "Resources/Shaders/";
		constexpr std::string_view MODELS_LOCATION = "Resources/Models/";
		constexpr std::string_view TEXTURES_LOCATION = "Resources/Textures/";
		int const WINDOW_WIDTH = 1800;
		int const WINDOW_HEIGHT = 900;
		float const PI = 3.14159265358979323846f;
	}
}