#pragma once
#include <tuple>

namespace Boom {
	namespace CONSTANTS {
		constexpr std::tuple<float, float, float, float> DEFAULT_BACKGROUND_COLOR = {.5f, .5f, .5f, 1.f};
		constexpr char const* SHADERS_LOCATION = "Resources/Shaders/";
		int const WINDOW_WIDTH = 1800;
		int const WINDOW_HEIGHT = 900;
	}
}