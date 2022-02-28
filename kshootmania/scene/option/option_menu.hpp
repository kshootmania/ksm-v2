#pragma once
#include "ui/linear_menu.hpp"
#include "graphics/texture_atlas.hpp"
#include "option_menu_field.hpp"

class OptionMenu
{
private:
	LinearMenu m_menu;

	Array<OptionMenuField> m_fields;

	const TextureAtlas m_fieldKeyTextureAtlas;

	const TextureAtlas m_fieldValueTextureAtlas;

	const Texture m_fieldCursorTexture;

	const Font m_font;

	Stopwatch m_stopwatch;

public:
	explicit OptionMenu(StringView fieldKeyTextureAssetKey, const Array<OptionMenuFieldCreateInfo>& fieldCreateInfos);

	void update();

	void draw(const Vec2& position) const;
};
