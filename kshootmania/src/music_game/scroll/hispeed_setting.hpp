#pragma once

namespace MusicGame
{
	/// @brief ハイスピードの種類
	enum class HispeedType : int32
	{
		XMod, // x-mod (倍率指定)
		OMod, // o-mod (最頻BPM基準での指定)
		CMod, // C-mod (定数指定)
		// KSMv1(HSP版)のa-mod、m-modは廃止
		EnumCount,
	};

	/// @brief ハイスピード設定
	struct HispeedSetting
	{
		/// @brief ハイスピードの種類
		HispeedType type = HispeedType::XMod;

		/// @brief ハイスピード値
		/// @remarks typeがXModの場合は倍率を10倍した値(1以上99以下の整数)。それ以外の場合はBPMに倍率をかけた後のハイスピード値(25以上2000以下)
		int32 value = 10;
	};
}
