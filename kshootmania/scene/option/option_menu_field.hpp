#pragma once
#include "ui/linear_menu.hpp"
#include "graphics/texture_atlas.hpp"

// TODO: Add IOptionMenuField interface, and OptionMenuIntField and OptionMenuEnumField that implement it

struct OptionMenuFieldCreateInfo
{
	String configIniKey;

	Array<std::pair<String, String>> valueDisplayNamePairs;

	OptionMenuFieldCreateInfo(StringView configIniKey, const Array<std::pair<String, String>>& valueDisplayNamePairs);

	OptionMenuFieldCreateInfo(StringView configIniKey, const Array<std::pair<int, String>>& valueDisplayNamePairs);

	OptionMenuFieldCreateInfo(StringView configIniKey, const Array<std::pair<double, String>>& valueDisplayNamePairs);
};

class OptionMenuField // TODO: rename as OptionMenuEnumField in future
{
private:
	const String m_configIniKey;

	const Array<std::pair<String, String>> m_valueDisplayNamePairs;

	LinearMenu m_menu;

public:
	explicit OptionMenuField(const OptionMenuFieldCreateInfo& createInfo);

	void update();

	void draw(const Vec2& position, const TextureRegion& keyTextureRegion, const TextureAtlas& valueTextureAtlas, const Font& font) const;
};
