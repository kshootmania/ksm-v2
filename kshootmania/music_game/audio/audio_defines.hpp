#pragma once

namespace MusicGame::Audio
{
	// 効果音は30ms早めのタイミングで再生する
	constexpr double kSELatencySec = 0.03;

	// LASER直角音の最大同時再生数
	// (旧バージョンの譜面では別の値を使う)
	constexpr uint32 kLaserSlamSEMaxPolyphony = 1U;
	constexpr uint32 kLaserSlamSEMaxPolyphonyLegacy = 20U;
	constexpr int32 kLaserSlamSEMaxPolyphonyLegacyUntilKSHVersion = 171;
}
