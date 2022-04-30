#include "jdgoverlay_3d_graphics.hpp"
#include "music_game/graphics/graphics_defines.hpp"
#include "ksh/common/common.hpp"

namespace
{
	using namespace MusicGame::Graphics;

	constexpr Size kTextureSize = { 452 * 2, 140 };

	constexpr Float3 kPlaneCenter = { 0.0f, 3.6f, -kHighwayPlaneSize.y / 2 - 1.8f };
	constexpr Float2 kPlaneSize = { kTextureSize.x * 0.95f / 8, kTextureSize.y * 0.95f / 8 };

	constexpr StringView kChipCriticalTextureFilename = U"judge1.gif";
	constexpr StringView kChipNearTextureFilename = U"judge2.gif";
	constexpr StringView kChipErrorTextureFilename = U"judge3.gif";
	constexpr int32 kChipAnimFrames = 12;
	constexpr double kChipAnimDurationSec = 0.5;
	constexpr Size kChipAnimSourceSize = { 150, 150 };
	constexpr Size kChipAnimSize = { 86, 86 };
}

const TiledTexture& MusicGame::Graphics::Jdgoverlay3DGraphics::chipTexture(Judgment::JudgmentResult type) const
{
	switch (type)
	{
		using enum Judgment::JudgmentResult;

	case kCritical:
		return m_chipCriticalTexture;

	case kNear:
		return m_chipNearTexture; // TODO: Fast near

	case kError:
	default:
		return m_chipErrorTexture;
	}
}

MusicGame::Graphics::Jdgoverlay3DGraphics::Jdgoverlay3DGraphics()
	: m_renderTexture(ScreenUtils::Scaled(kTextureSize.x), ScreenUtils::Scaled(kTextureSize.y))
	, m_renderTexture2(ScreenUtils::Scaled(kTextureSize.x), ScreenUtils::Scaled(kTextureSize.y))
	, m_mesh(MeshData::Grid(kPlaneCenter, kPlaneSize, 2, 2))
	, m_chipCriticalTexture(kChipCriticalTextureFilename,
		{
			.row = kChipAnimFrames,
			.sourceScale = ScreenUtils::SourceScale::kNoScaling,
			.sourceSize = kChipAnimSourceSize,
		})
	, m_chipNearTexture(kChipNearTextureFilename,
		{
			.row = kChipAnimFrames,
			.column = 2, // Column#0: Slow (or Default), Column#1: Fast
			.sourceScale = ScreenUtils::SourceScale::kNoScaling,
			.sourceSize = kChipAnimSourceSize,
		})
	, m_chipErrorTexture(kChipErrorTextureFilename,
		{
			.row = kChipAnimFrames,
			.sourceScale = ScreenUtils::SourceScale::kNoScaling,
			.sourceSize = kChipAnimSourceSize,
		})
{
}

void MusicGame::Graphics::Jdgoverlay3DGraphics::draw(const UpdateInfo& updateInfo, double tiltRadians) const
{
	{
		Shader::Copy(m_renderTexture2, m_renderTexture); // FIXME: For some reason, RenderTexture::clear() hides the animation, so for now using Shader::Copy() instead.
		const ScopedRenderTarget2D renderTarget(m_renderTexture/*.clear(Palette::Blue)*/);
		const ScopedRenderStates2D blendState(BlendState::Additive);
		for (std::size_t i = 0; i < ksh::kNumBTLanes; ++i)
		{
			const auto& laneState = updateInfo.btLaneState[i];
			for (const auto& chipAnimState : laneState.chipAnimStateRingBuffer)
			{
				const double sec = updateInfo.currentTimeSec - chipAnimState.startTimeSec;
				if (0.0 <= sec && sec < kChipAnimDurationSec && chipAnimState.type != Judgment::JudgmentResult::kUnspecified)
				{
					const int32 frameIdx = static_cast<int32>(sec / kChipAnimDurationSec * kChipAnimFrames);
					chipTexture(chipAnimState.type)(frameIdx).resized(ScreenUtils::Scaled(kChipAnimSize)).draw(ScreenUtils::Scaled(kTextureSize.x / 4 + 92 + 60 * i, 17));
				}
			}
		}
	}

	const ScopedRenderStates3D blendState(BlendState::Additive);
	const Mat4x4 m = Mat4x4::Rotate(Float3::Right(), -60_deg, kPlaneCenter) * Mat4x4::Rotate(Float3::Backward(), tiltRadians, Vec3{ 0.0, 42.0, 0.0 });
	const Transformer3D transform{ m };
	m_mesh.draw(m_renderTexture);
}
