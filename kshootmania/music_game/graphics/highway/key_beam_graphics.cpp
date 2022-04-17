#include "key_beam_graphics.hpp"
#include "music_game/graphics/graphics_defines.hpp"

namespace
{
	using namespace MusicGame::Graphics;

	constexpr StringView kKeyBeamTextureFilename = U"judge.gif";

	constexpr Size kBTKeyBeamTextureSize = { 40, 300 };
	constexpr Size kFXKeyBeamTextureSize = { 82, 300 };

	constexpr double kKeyBeamFullWidthSec = 0.075;
	constexpr double kKeyBeamEndSec = 0.155;
	constexpr Vec2 kKeyBeamPositionOffset = kLanePositionOffset + Vec2{ 0.0, kHighwayTextureSize.y - 300.0 };
}

MusicGame::Graphics::KeyBeamGraphics::KeyBeamGraphics()
	: m_beamTexture(TextureAsset(kKeyBeamTextureFilename))
{
}

void MusicGame::Graphics::KeyBeamGraphics::draw(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget) const
{
	const ScopedRenderTarget2D renderTarget(additiveTarget);
	const ScopedRenderStates2D renderState(BlendState::Additive);

	for (std::size_t i = 0; i < ksh::kNumBTLanes + ksh::kNumFXLanes; ++i)
	{
		const bool isBT = (i < ksh::kNumBTLanes);
		const std::size_t laneIdx = isBT ? i : (i - ksh::kNumBTLanes);
		const LaneState& laneState = isBT ? updateInfo.btLaneState[laneIdx] : updateInfo.fxLaneState[laneIdx];

		const double sec = updateInfo.currentTimeSec - laneState.keyBeamTimeSec;
		if (sec < 0.0 || kKeyBeamEndSec < sec)
		{
			continue;
		}

		double widthRate = 1.0;
		double alpha = 1.0;
		if (sec < kKeyBeamFullWidthSec)
		{
			widthRate = sec / kKeyBeamFullWidthSec;
		}
		else
		{
			alpha = 1.0 - (sec - kKeyBeamFullWidthSec) / (kKeyBeamEndSec - kKeyBeamFullWidthSec);
		}

		const TextureRegion beamTextureRegion = m_beamTexture(
			kBTKeyBeamTextureSize.x * (laneState.keyBeamType + 0.5 - widthRate / 2),
			0,
			kBTKeyBeamTextureSize.x * widthRate,
			kBTKeyBeamTextureSize.y);

		if (isBT)
		{
			beamTextureRegion
				.draw(kKeyBeamPositionOffset + kBTLanePositionDiff * (laneIdx + 0.5 - widthRate / 2), ColorF{ 1.0, alpha });
		}
		else
		{
			beamTextureRegion
				.resized(kFXKeyBeamTextureSize.x * widthRate, kFXKeyBeamTextureSize.y)
				.draw(kKeyBeamPositionOffset + kFXLanePositionDiff * laneIdx + kFXKeyBeamTextureSize * (0.5 - widthRate / 2), ColorF{ 1.0, alpha });
		}
	}
}
