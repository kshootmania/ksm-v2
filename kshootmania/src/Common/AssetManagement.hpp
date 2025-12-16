#pragma once

namespace AssetManagement
{
	void RegisterAssets();

	void RefreshSystemFontForCurrentLanguage();

	[[nodiscard]]
	Font SystemFont();

	[[nodiscard]]
	Font SystemFontBold();
}
