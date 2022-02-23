#pragma once
#include <Siv3D.hpp>

namespace ScreenUtils
{
	TextureRegion FitToHeight(const Texture& texture);

	constexpr int32 kLegacyTextureBaseScreenHeight = 480;

	inline int32 Scaled(const int32 v)
	{
		const Size screenSize = Scene::Size();
		return v * screenSize.y / kLegacyTextureBaseScreenHeight;
	}

	inline Point Scaled(const Point & point)
	{
		return { Scaled(point.x), Scaled(point.y) };
	}

	inline Point Scaled(const int32 x, const int32 y)
	{
		return { Scaled(x), Scaled(y) };
	}

	inline TextureRegion Scaled(const Texture& texture)
	{
		return texture.scaled(Scaled(texture.width(), texture.height()));
	}

	inline TextureRegion Scaled(const TextureRegion& textureRegion)
	{
		int32 w = Scaled(static_cast<int32>(textureRegion.size.x));
		int32 h = Scaled(static_cast<int32>(textureRegion.size.y));
		return textureRegion.resized(w, h);
	}

	// Scaling functions for 3x-resolution textures

	constexpr int32 kScaleLNumerator = 1;
	constexpr int32 kScaleLDenominator = 3;

	inline int32 ScaledL(const int32 v)
	{
		const Size screenSize = Scene::Size();
		return v * screenSize.y * kScaleLNumerator / kLegacyTextureBaseScreenHeight / kScaleLDenominator;
	}

	inline Point ScaledL(const Point& point)
	{
		return { ScaledL(point.x), ScaledL(point.y) };
	}

	inline Point ScaledL(const int32 x, const int32 y)
	{
		return { ScaledL(x), ScaledL(y) };
	}

	inline TextureRegion ScaledL(const TextureRegion& textureRegion)
	{
		int32 w = ScaledL(static_cast<int32>(textureRegion.size.x));
		int32 h = ScaledL(static_cast<int32>(textureRegion.size.y));
		return textureRegion.resized(w, h);
	}

	// Scaling functions for high resolution textures
	// (Historical note: This scale was determined based on the display size of msel_0.png (original width:766px))

	constexpr int32 kScaleLLNumerator = 225;
	constexpr int32 kScaleLLDenominator = 766;

	inline int32 ScaledLL(const int32 v)
	{
		const Size screenSize = Scene::Size();
		return v * screenSize.y * kScaleLLNumerator / kLegacyTextureBaseScreenHeight / kScaleLLDenominator;
	}

	inline Point ScaledLL(const Point& point)
	{
		return { ScaledLL(point.x), ScaledLL(point.y) };
	}

	inline Point ScaledLL(const int32 x, const int32 y)
	{
		return { ScaledLL(x), ScaledLL(y) };
	}

	inline TextureRegion ScaledLL(const TextureRegion& textureRegion)
	{
		int32 w = ScaledLL(static_cast<int32>(textureRegion.size.x));
		int32 h = ScaledLL(static_cast<int32>(textureRegion.size.y));
		return textureRegion.resized(w, h);
	}
}
