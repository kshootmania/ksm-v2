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
	using enum Judgment::JudgmentResult;

	switch (type)
	{
	case kCritical:
		return m_chipCriticalTexture;

	case kNear:
		return m_chipNearTexture; // TODO: Fast near

	case kError:
		return m_chipErrorTexture;

	default:
		assert(false);
		return m_chipErrorTexture;
	}
}

void MusicGame::Graphics::Jdgoverlay3DGraphics::drawChipAnimCommon(const UpdateInfo& updateInfo, bool isBT) const
{
	for (int32 i = 0; std::cmp_less(i, (isBT ? ksh::kNumBTLanes : ksh::kNumFXLanes)); ++i)
	{
		const auto& laneState = isBT ? updateInfo.btLaneState[i] : updateInfo.fxLaneState[i];
		for (const auto& chipAnimState : laneState.chipAnimStateRingBuffer)
		{
			const double sec = updateInfo.currentTimeSec - chipAnimState.startTimeSec;
			if (0.0 <= sec && sec < kChipAnimDurationSec && chipAnimState.type != Judgment::JudgmentResult::kUnspecified)
			{
				const int32 frameIdx = static_cast<int32>(sec / kChipAnimDurationSec * kChipAnimFrames);
				const Vec2 position = ScreenUtils::Scaled(kTextureSize.x / 4 + 92 + (isBT ? 0 : 30) + 60 * i * (isBT ? 1 : 2), 17);
				chipTexture(chipAnimState.type)(frameIdx).resized(ScreenUtils::Scaled(kChipAnimSize)).draw(position);
			}
		}
	}
}

void MusicGame::Graphics::Jdgoverlay3DGraphics::drawChipAnimBT(const UpdateInfo& updateInfo) const
{
	drawChipAnimCommon(updateInfo, true);
}

void MusicGame::Graphics::Jdgoverlay3DGraphics::drawChipAnimFX(const UpdateInfo& updateInfo) const
{
	drawChipAnimCommon(updateInfo, false);
}

MusicGame::Graphics::Jdgoverlay3DGraphics::Jdgoverlay3DGraphics()
	: m_renderTexture(ScreenUtils::Scaled(kTextureSize.x), ScreenUtils::Scaled(kTextureSize.y))
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

void MusicGame::Graphics::Jdgoverlay3DGraphics::draw2D(const UpdateInfo& updateInfo) const
{
	// Prepare 2D render texture to draw
	const ScopedRenderTarget2D renderTarget(m_renderTexture.clear(Palette::Black));
	const ScopedRenderStates2D blendState(BlendState::Additive);
	drawChipAnimBT(updateInfo);
	drawChipAnimFX(updateInfo);
}

void MusicGame::Graphics::Jdgoverlay3DGraphics::draw3D(double tiltRadians) const
{
	// Draw 2D render texture into 3D plane
	const ScopedRenderStates3D blendState(BlendState::Additive);
	const Transformer3D transform(JudgmentPlaneTransformMatrix(tiltRadians, kPlaneCenter));
	m_mesh.draw(m_renderTexture);
}
