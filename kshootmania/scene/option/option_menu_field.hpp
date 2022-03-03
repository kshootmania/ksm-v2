#pragma once
#include "ui/linear_menu.hpp"
#include "graphics/texture_atlas.hpp"

// TODO: Add IOptionMenuField interface, and OptionMenuIntField and OptionMenuEnumField that implement it

struct OptionMenuFieldCreateInfo
{
	String configIniKey;

	// For non-enum
	int32 valueMin = 0;
	int32 valueMax = 0;
	int32 valueStep = 0;
	String suffixStr;

	// For enum
	Array<std::pair<String, String>> valueDisplayNamePairs;

	OptionMenuFieldCreateInfo(StringView configIniKey, const Array<String>& valueDisplayNames);

	OptionMenuFieldCreateInfo(StringView configIniKey, const Array<StringView>& valueDisplayNames);

	OptionMenuFieldCreateInfo(StringView configIniKey, const Array<std::pair<String, String>>& valueDisplayNamePairs);

	OptionMenuFieldCreateInfo(StringView configIniKey, const Array<std::pair<int, String>>& valueDisplayNamePairs);

	OptionMenuFieldCreateInfo(StringView configIniKey, const Array<std::pair<double, String>>& valueDisplayNamePairs);

	OptionMenuFieldCreateInfo(StringView configIniKey, int32 valueMin, int32 valueMax, StringView suffixStr = U"", int32 valueStep = 1);
};

class OptionMenuField
{
private:
	const String m_configIniKey;

	bool m_isEnum;

	// For non-enum
	const String m_suffixStr;

	// For enum
	const Array<std::pair<String, String>> m_valueDisplayNamePairs;

	LinearMenu m_menu;

public:
	explicit OptionMenuField(const OptionMenuFieldCreateInfo& createInfo);

	void update();

	void draw(const Vec2& position, const TextureRegion& keyTextureRegion, const TextureAtlas& valueTextureAtlas, const Font& font) const;
};
