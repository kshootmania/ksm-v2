#pragma once

inline namespace AssetManagement
{
	constexpr StringView kAssetPrefixSRGB = U"SRGB:";

	constexpr StringView kFontAssetSystem = U"System";
	constexpr StringView kFontAssetSystemBold = U"SystemBold";
}

namespace AssetManagement
{
	void RegisterAssets();
}
