#include "achievement_panel.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr double kStartSec = 0.8;
		constexpr double kAlphaFadeInDurationSec = 0.64;
		constexpr int32 kSizeXStart = 135;
		constexpr int32 kSizeXFinish = 270;
		constexpr int32 kSizeYStart = 25;
		constexpr int32 kSizeYFinish = 90;
		constexpr double kScaleXDurationSec = 0.405;
		constexpr double kScaleYDurationSec = 0.39;

		int32 GetAchievementRow(Achievement achievement)
		{
			return static_cast<int32>(achievement);
		}
	}

	AchievementPanel::AchievementPanel()
		: m_texture(U"playresult.png", TiledTextureSizeInfo
			{
				.row = 4,
				.sourceScale = SourceScale::k2x,
				.sourceSize = { 540, 150 },
			})
	{
	}

	void AchievementPanel::draw(const GameStatus& gameStatus) const
	{
		if (!gameStatus.playFinishStatus.has_value())
		{
			// まだプレイが終了していない場合は何も表示しない
			return;
		}

		const PlayFinishStatus& playFinishStatus = gameStatus.playFinishStatus.value();
		const double secSincePlayFinish = gameStatus.currentTimeSec - playFinishStatus.finishTimeSec;
		if (secSincePlayFinish < kStartSec)
		{
			// プレイ終了後一定時間は何も表示しない
			return;
		}

		// プレイ結果を表示
		const double sec = secSincePlayFinish - kStartSec;
		const double alpha = Min(sec / kAlphaFadeInDurationSec, 1.0);
		const double scaleXRate = Min(sec / kScaleXDurationSec, 1.0);
		const double sizeX = Math::Lerp(Scaled(kSizeXStart), Scaled(kSizeXFinish), scaleXRate);
		const double scaleYRate = Min(sec / kScaleYDurationSec, 1.0);
		const double sizeY = Math::Lerp(Scaled(kSizeYStart), Scaled(kSizeYFinish), scaleYRate);
		const int32 row = GetAchievementRow(playFinishStatus.achievement);
		const Point pos = Scene::Center() - Scaled(0, 40);
		m_texture(row).resized(sizeX, sizeY).drawAt(pos, ColorF(1.0, alpha));
	}
}
