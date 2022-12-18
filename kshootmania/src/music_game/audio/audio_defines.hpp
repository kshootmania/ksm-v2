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

	// LASER直角音のデフォルト音量
	// (譜面データに音量指定が1つもない場合に使用)
	constexpr double kLaserSlamDefaultVolume = 0.5f;
}
