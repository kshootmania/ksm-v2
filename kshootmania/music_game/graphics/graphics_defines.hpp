#pragma once

namespace MusicGame::Graphics
{
	constexpr Size kHighwayTextureSize = { 256, 1024 };
	constexpr Float2 kHighwayPlaneSize = { 304.0f * 2 / 13, 936.0f * 13 / 20 };

	constexpr Vec2 kLanePositionOffset = { 44.0, 0.0 };
	constexpr Vec2 kBTLanePositionDiff = { 42.0, 0.0 };
	constexpr Vec2 kFXLanePositionDiff = { 84.0, 0.0 };

	// Note: If you use this blend state, make sure the destination texture is opaque.
	//       Otherwise, both alpha and color blending will take effect and the drawn image will be dark.
	constexpr BlendState kEnableAlphaBlend = BlendState(
		true,
		Blend::SrcAlpha,
		Blend::InvSrcAlpha,
		BlendOp::Add,
		Blend::One);

	constexpr BlendState kInvMultiply = BlendState(
		true,
		Blend::Zero,
		Blend::InvSrcColor,
		BlendOp::Add,
		Blend::Zero,
		Blend::One,
		BlendOp::Add);

	constexpr ColorF kTransparent = ColorF{ 0.0, 0.0 };

	constexpr ColorF kSongInfoFontColor = Color{ 0, 32, 8 };
}
