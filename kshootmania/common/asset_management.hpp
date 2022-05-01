#pragma once

namespace AssetManagement
{
	constexpr StringView kAssetPrefixSRGB = U"SRGB:";

	void RegisterAssets();
}

using AssetManagement::kAssetPrefixSRGB;
