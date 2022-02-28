#pragma once
#include <Siv3D.hpp>

namespace ScreenUtils
{
	TextureRegion FitToHeight(const Texture& texture);

	constexpr int32 kBaseScreenWidth = 640;
	constexpr int32 kBaseScreenHeight = 480;

	inline int32 Scaled(const int32 v)
	{
		const Size screenSize = Scene::Size();
		return v * screenSize.y / kBaseScreenHeight;
	}

	inline int32 ScaledByWidth(const int32 v)
	{
		const Size screenSize = Scene::Size();
		return v * screenSize.x / kBaseScreenWidth;
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

	inline int32 LeftMargin()
	{
		const Size screenSize = Scene::Size();
		return (screenSize.x - screenSize.y * kBaseScreenWidth / kBaseScreenHeight) / 2;
	}

	inline Vec2 LeftMarginVec()
	{
		return Vec2{ LeftMargin(), 0.0 };
	}

	// Scaling functions for 2x-resolution textures

	constexpr int32 kScale2xNumerator = 1;
	constexpr int32 kScale2xDenominator = 2;

	inline int32 Scaled2x(const int32 v)
	{
		const Size screenSize = Scene::Size();
		return v * screenSize.y * kScale2xNumerator / kBaseScreenHeight / kScale2xDenominator;
	}

	inline Point Scaled2x(const Point& point)
	{
		return { Scaled2x(point.x), Scaled2x(point.y) };
	}

	inline Point Scaled2x(const int32 x, const int32 y)
	{
		return { Scaled2x(x), Scaled2x(y) };
	}

	inline TextureRegion Scaled2x(const TextureRegion& textureRegion)
	{
		int32 w = Scaled2x(static_cast<int32>(textureRegion.size.x));
		int32 h = Scaled2x(static_cast<int32>(textureRegion.size.y));
		return textureRegion.resized(w, h);
	}

	// Scaling functions for 3x-resolution textures

	constexpr int32 kScale3xNumerator = 1;
	constexpr int32 kScale3xDenominator = 3;

	inline int32 Scaled3x(const int32 v)
	{
		const Size screenSize = Scene::Size();
		return v * screenSize.y * kScale3xNumerator / kBaseScreenHeight / kScale3xDenominator;
	}

	inline Point Scaled3x(const Point& point)
	{
		return { Scaled3x(point.x), Scaled3x(point.y) };
	}

	inline Point Scaled3x(const int32 x, const int32 y)
	{
		return { Scaled3x(x), Scaled3x(y) };
	}

	inline TextureRegion Scaled3x(const TextureRegion& textureRegion)
	{
		int32 w = Scaled3x(static_cast<int32>(textureRegion.size.x));
		int32 h = Scaled3x(static_cast<int32>(textureRegion.size.y));
		return textureRegion.resized(w, h);
	}

	// Scaling functions for high resolution textures
	// (Historical note: This scale was determined based on the display size of msel_0.png (original width:766px))

	constexpr int32 kScaleLNumerator = 225;
	constexpr int32 kScaleLDenominator = 766;

	inline int32 ScaledL(const int32 v)
	{
		const Size screenSize = Scene::Size();
		return v * screenSize.y * kScaleLNumerator / kBaseScreenHeight / kScaleLDenominator;
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
}
