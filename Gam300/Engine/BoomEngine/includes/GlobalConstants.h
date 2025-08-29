#pragma once
#include <tuple>

namespace Boom {
	namespace CONSTANTS {
		constexpr std::tuple<float, float, float, float> DEFAULT_BACKGROUND_COLOR = {.3f, .3f, .3f, 1.f};
		constexpr char const* SHADERS_LOCATION = "Resources/Shaders/";
		constexpr char const* MODELS_LOCAITON = "Resources/Models/";
		int const WINDOW_WIDTH = 1800;
		int const WINDOW_HEIGHT = 900;
		float const PI = 3.14159265358979323846f;
	}
}