#include "KeyBeamGraphics.hpp"
#include "MusicGame/Graphics/GraphicsDefines.hpp"
#include "MusicGame/Camera/CameraMath.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kKeyBeamTextureFilename = U"judge.gif";
		constexpr StringView kKeyBeamWipeMaskTextureFilename = U"judge_wipe_mask.gif";

		constexpr Size kBTKeyBeamTextureSize = { 40, 300 };
		constexpr Size kFXKeyBeamTextureSize = { 82, 300 };

		constexpr double kKeyBeamFullWidthSec = 0.075;
		constexpr double kKeyBeamEndSec = 0.155;
		constexpr Vec2 kKeyBeamPositionOffset = kLanePositionOffset + Vec2{ 0.0, kHighwayTextureSize.y - 300.0 };

		Texture CreateWipeMaskTexture(StringView filename)
		{
			const FilePath filePath = FileSystem::PathAppend(FsUtils::GetResourcePath(U"imgs"), filename);
			return FileSystem::Exists(filePath) ? Texture{ filePath } : Texture{};
		}
	}

	KeyBeamGraphics::KeyBeamGraphics()
		: m_beamTexture(TextureAsset(kKeyBeamTextureFilename))
		, m_beamWipeMaskTexture(CreateWipeMaskTexture(kKeyBeamWipeMaskTextureFilename))
	{
	}

	void KeyBeamGraphics::draw(const GameStatus& gameStatus, const ViewStatus& viewStatus, const HighwayRenderTexture& target) const
	{
		const ScopedRenderStates2D renderState(BlendState::Additive);

		// レーン消失の範囲ではビームの背面が暗くならないため、invMultiply側へマスクを描画
		const HighwayWipeRegion wipeRegion = CalcHighwayWipeRegion(viewStatus.camStatus.highwayWipe);
		const int32 numPasses = (wipeRegion.wipeHeight > 0 && !m_beamWipeMaskTexture.isEmpty()) ? 2 : 1;

		// レンダーターゲットの切り替えはレーン単位で行うと描画バッチが分断されるため、パス単位でまとめる
		for (int32 pass = 0; pass < numPasses; ++pass)
		{
			const bool isWipeMask = pass == 1;
			const ScopedRenderTarget2D renderTarget(isWipeMask ? target.invMultiplyTexture() : target.additiveTexture());

			for (std::size_t i = 0; i < kson::kNumBTLanesSZ + kson::kNumFXLanesSZ; ++i)
			{
				const bool isBT = (i < kson::kNumBTLanesSZ);
				const std::size_t numLanes = isBT ? kson::kNumBTLanesSZ : kson::kNumFXLanesSZ;
				const std::size_t laneIdx = isBT ? i : (i - kson::kNumBTLanesSZ);
				const double centerSplitShiftX = Camera::CenterSplitShiftX(viewStatus.camStatus.centerSplit) * ((laneIdx >= numLanes / 2) ? 1 : -1);
				const ButtonLaneStatus& laneStatus = isBT ? gameStatus.btLaneStatus[laneIdx] : gameStatus.fxLaneStatus[laneIdx];

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

				// マスクの画像はビームの種類によらず1列のみ
				const int32 textureColumn = isWipeMask ? 0 : static_cast<int32>(laneStatus.keyBeamType);
				const TextureRegion beamTextureRegion = (isWipeMask ? m_beamWipeMaskTexture : m_beamTexture)(
					kBTKeyBeamTextureSize.x * (static_cast<double>(textureColumn) + 0.5 - widthRate / 2),
					0,
					kBTKeyBeamTextureSize.x * widthRate,
					kBTKeyBeamTextureSize.y);

				const double dLaneIdx = static_cast<double>(laneIdx);

				const TextureRegion region = isBT ? beamTextureRegion : beamTextureRegion.resized(kFXKeyBeamTextureSize.x * widthRate, kFXKeyBeamTextureSize.y);
				const Vec2 position = isBT
					? kKeyBeamPositionOffset + Vec2::Right(centerSplitShiftX) + kBTLanePositionDiff * (dLaneIdx + 0.5 - widthRate / 2)
					: kKeyBeamPositionOffset + Vec2::Right(centerSplitShiftX) + kFXLanePositionDiff * dLaneIdx + kFXKeyBeamTextureSize * (0.5 - widthRate / 2);
				if (isWipeMask)
				{
					DrawInHighwayWipeRegion(region, position, wipeRegion, ColorF{ 1.0, alpha });
				}
				else
				{
					region.draw(position, ColorF{ 1.0, alpha });
				}
			}
		}
	}
}
