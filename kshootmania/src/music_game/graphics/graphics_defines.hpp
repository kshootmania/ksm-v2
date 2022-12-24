#pragma once

namespace MusicGame::Graphics
{
	constexpr Size kHighwayTextureSize = { 256, 1024 };
	constexpr Float2 kHighwayPlaneSize = { 304.0f * 2 / 13, 936.0f * 13 / 20 };

	constexpr Size kHighwayTextureSizeWide = { kHighwayTextureSize.x * 2, kHighwayTextureSize.y };
	constexpr Float2 kHighwayPlaneSizeWide = { kHighwayPlaneSize.x * 2, kHighwayPlaneSize.y };

	constexpr Vec2 kLanePositionOffset = { 44.0, 0.0 };
	constexpr Vec2 kBTLanePositionDiff = { 42.0, 0.0 };
	constexpr Vec2 kFXLanePositionDiff = { 84.0, 0.0 };

	// アルファブレンド有効で描画
	// (描画先テクスチャはあらかじめ不透明にしておく必要があるので注意。不透明でない場合、見た目が暗くなる)
	constexpr BlendState kEnableAlphaBlend = BlendState(
		true,
		Blend::SrcAlpha,
		Blend::InvSrcAlpha,
		BlendOp::Add,
		Blend::One);

	// 反転・乗算描画
	// (HSP版ではhgimg3の「色減算(substract2)」を使用していたが、このモードはなぜか減算ではなく反転・乗算しているようだったのでこちらを使用)
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

	constexpr double kOnePixelTextureSourceSize = 0.5; // 幅1pxのテクスチャを幅1pxとして使用すると縁に余白ができるので、描画元サイズは正確に1pxにせず小さめにする

	constexpr double kOnePixelTextureSourceOffset = (1.0 - kOnePixelTextureSourceSize) / 2; // kOnePixelTextureSourceSize使用時の座標オフセット

	constexpr Vec3 kHighwayTiltCenter = { 0.0, 42.0, 0.0 };

	inline Mat4x4 TiltTransformMatrix(double radians, const Vec3& center = kHighwayTiltCenter)
	{
		return Mat4x4::Rotate(Float3::Backward(), radians, center);
	}

	inline Mat4x4 JudgmentPlaneTransformMatrix(double tiltRadians, const Vec3& planeCenter)
	{
		return Mat4x4::Rotate(Float3::Right(), -60_deg, planeCenter) * TiltTransformMatrix(tiltRadians);
	}

	constexpr std::size_t kChipAnimMax = 5U;
}
