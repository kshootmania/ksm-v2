#pragma once

namespace MusicGame::Audio
{
	// 効果音のレイテンシ補正用のオフセット
	// (Windowsは70ms、それ以外は35ms早めに再生する)
#ifdef _WIN32
	constexpr double kSELatencySec = 0.070;
#else
	constexpr double kSELatencySec = 0.035;
#endif

	// LASER直角音の最大同時再生数
	// (旧バージョンの譜面では別の値を使う)
	constexpr uint32 kLaserSlamSEMaxPolyphony = 1U;
	constexpr uint32 kLaserSlamSEMaxPolyphonyLegacy = 20U;
	constexpr int32 kLaserSlamSEMaxPolyphonyLegacyUntilKSHVersion = 171;

	// FXチップノーツの同一効果音の最大同時再生数
	// (旧バージョンの譜面では別の値を使う)
	constexpr uint32 kKeySoundMaxPolyphony = 1U;
	constexpr uint32 kKeySoundMaxPolyphonyLegacy = 10U;
	constexpr int32 kKeySoundMaxPolyphonyLegacyUntilKSHVersion = 171;

	// LASER直角音のデフォルト音量
	// (譜面データに音量指定が1つもない場合に使用)
	constexpr double kLaserSlamDefaultVolume = 0.5f;
}
