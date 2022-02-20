#include "screen_utils.hpp"

TextureRegion ScreenUtils::FitToHeight(const Texture& texture)
{
	const double height = Scene::Height();
	const double width = texture.width() * height / texture.height();
	return texture.resized(width, height);
}
