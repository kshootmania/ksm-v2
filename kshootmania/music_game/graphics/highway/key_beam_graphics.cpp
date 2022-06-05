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

void MusicGame::Graphics::KeyBeamGraphics::draw(const GameStatus& gameStatus, const RenderTexture& additiveTarget) const
{
	const ScopedRenderTarget2D renderTarget(additiveTarget);
	const ScopedRenderStates2D renderState(BlendState::Additive);

	for (std::size_t i = 0; i < kson::kNumBTLanes + kson::kNumFXLanes; ++i)
	{
		const bool isBT = (i < kson::kNumBTLanes);
		const std::size_t laneIdx = isBT ? i : (i - kson::kNumBTLanes);
		const LaneStatus& laneStatus = isBT ? gameStatus.btLaneStatus[laneIdx] : gameStatus.fxLaneStatus[laneIdx];

		const double sec = gameStatus.currentTimeSec - laneStatus.keyBeamTimeSec;
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
			kBTKeyBeamTextureSize.x * (static_cast<double>(static_cast<int32>(laneStatus.keyBeamType)) + 0.5 - widthRate / 2),
			0,
			kBTKeyBeamTextureSize.x * widthRate,
			kBTKeyBeamTextureSize.y);

		const double dLaneIdx = static_cast<double>(laneIdx);
		
		if (isBT)
		{
			beamTextureRegion
				.draw(kKeyBeamPositionOffset + kBTLanePositionDiff * (dLaneIdx + 0.5 - widthRate / 2), ColorF{ 1.0, alpha });
		}
		else
		{
			beamTextureRegion
				.resized(kFXKeyBeamTextureSize.x * widthRate, kFXKeyBeamTextureSize.y)
				.draw(kKeyBeamPositionOffset + kFXLanePositionDiff * dLaneIdx + kFXKeyBeamTextureSize * (0.5 - widthRate / 2), ColorF{ 1.0, alpha });
		}
	}
}
