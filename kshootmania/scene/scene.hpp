#pragma once
#include "title/title_scene.hpp"
#include "option/option_scene.hpp"
#include "select/select_scene.hpp"

namespace SceneName
{
	constexpr StringView kTitle = U"Title";
	constexpr StringView kOption = U"Option";
	constexpr StringView kSelect = U"Select";
}

constexpr int32 kDefaultTransitionMs = 800;
