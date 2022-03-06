#include "option_menu_field.hpp"
#include "ui/menu_helper.hpp"
#include "ini/config_ini.hpp"

namespace
{
	template <typename T>
	Array<std::pair<String, String>> ConvertValueDisplayNamePairs(const Array<std::pair<T, String>>& valueDisplayNamePairs)
	{
		return valueDisplayNamePairs.map([](const std::pair<T, String>& pair) {
			return std::pair<String, String>{ Format(pair.first), pair.second };
		});
	}

	Array<std::pair<String, String>> ConvertValueDisplayNamePairs(const Array<String>& valueDisplayNames)
	{
		Array<std::pair<String, String>> pairs;
		for (std::size_t i = 0; i < valueDisplayNames.size(); ++i)
		{
			pairs.emplace_back(Format(i), valueDisplayNames[i]);
		}
		return pairs;
	}

	Array<std::pair<String, String>> ConvertValueDisplayNamePairs(const Array<StringView>& valueDisplayNames)
	{
		Array<std::pair<String, String>> pairs;
		for (std::size_t i = 0; i < valueDisplayNames.size(); ++i)
		{
			pairs.emplace_back(Format(i), valueDisplayNames[i]);
		}
		return pairs;
	}

	LinearMenu MakeMenuInt(int32 valueMin, int32 valueMax, int32 valueDefault, int32 valueStep)
	{
		return MenuHelper::MakeHorizontalMenuWithMinMax(valueMin, valueMax, MenuHelper::ButtonFlags::kArrowOrBTOrFXOrLaser, IsCyclicMenu::No, 0.0, valueDefault, valueStep);
	}

	LinearMenu MakeMenuEnum(int32 enumCount, int32 valueDefault)
	{
		return MenuHelper::MakeHorizontalMenu(enumCount, MenuHelper::ButtonFlags::kArrowOrBTOrFXOrLaser, IsCyclicMenu::No, 0.0, valueDefault);
	}

	OptionMenuField::ArrowType GetMenuFieldValueArrowType(const LinearMenu& menu)
	{
		const bool isMin = menu.isCursorMin();
		const bool isMax = menu.isCursorMax();

		if (isMin && isMax)
		{
			return OptionMenuField::kArrowTypeNone;
		}

		if (isMin)
		{
			return OptionMenuField::kArrowTypeRight;
		}

		if (isMax)
		{
			return OptionMenuField::kArrowTypeLeft;
		}

		return OptionMenuField::kArrowTypeLeftRight;
	}

	int32 FindDefaultCursor(StringView configIniKey, bool isEnum, const Array<std::pair<String, String>>& valueDisplayNamePairs, int32 cursorDefault)
	{
		if (ConfigIni::HasValue(configIniKey))
		{
			if (isEnum)
			{
				const StringView value = ConfigIni::GetString(configIniKey);
				for (std::size_t i = 0; i < valueDisplayNamePairs.size(); ++i)
				{
					if (valueDisplayNamePairs[i].first == value)
					{
						return static_cast<int32>(i);
					}
				}
			}
			else
			{
				return ConfigIni::GetInt(configIniKey, cursorDefault);
			}
		}

		return cursorDefault;
	}
}

OptionMenuFieldCreateInfo OptionMenuFieldCreateInfo::Enum(StringView configIniKey, const Array<String>& valueDisplayNames)
{
	return Enum(configIniKey, ConvertValueDisplayNamePairs(valueDisplayNames));
}

OptionMenuFieldCreateInfo OptionMenuFieldCreateInfo::Enum(StringView configIniKey, const Array<StringView>& valueDisplayNames)
{
	return Enum(configIniKey, ConvertValueDisplayNamePairs(valueDisplayNames));
}

OptionMenuFieldCreateInfo OptionMenuFieldCreateInfo::Enum(StringView configIniKey, const Array<std::pair<String, String>>& valueDisplayNamePairs)
{
	return OptionMenuFieldCreateInfo{
		.configIniKey = String(configIniKey),
		.valueDisplayNamePairs = valueDisplayNamePairs,
	};
}

OptionMenuFieldCreateInfo OptionMenuFieldCreateInfo::Enum(StringView configIniKey, const Array<std::pair<int, String>>& valueDisplayNamePairs)
{
	return Enum(configIniKey, ConvertValueDisplayNamePairs(valueDisplayNamePairs));
}

OptionMenuFieldCreateInfo OptionMenuFieldCreateInfo::Enum(StringView configIniKey, const Array<std::pair<double, String>>& valueDisplayNamePairs)
{
	return Enum(configIniKey, ConvertValueDisplayNamePairs(valueDisplayNamePairs));
}

OptionMenuFieldCreateInfo OptionMenuFieldCreateInfo::Int(StringView configIniKey, int32 valueMin, int32 valueMax, int32 valueDefault, StringView suffixStr, int32 valueStep)
{
	if (valueStep == 0)
	{
		// CreateInfo having zero valueStep is misrecognized as an enum field, so throw an exception.
		throw Error(U"OptionMenuFieldCreateInfo::Int(): valueStep must not be zero!");
	}

	return OptionMenuFieldCreateInfo{
		.configIniKey = String(configIniKey),
		.valueMin = valueMin,
		.valueMax = valueMax,
		.valueDefault = valueDefault,
		.valueStep = valueStep,
		.suffixStr = String(suffixStr),
	};
}

OptionMenuFieldCreateInfo& OptionMenuFieldCreateInfo::setKeyTextureIdx(int32 idx)&
{
	keyTextureIdx = idx;
	return *this;
}

OptionMenuFieldCreateInfo&& OptionMenuFieldCreateInfo::setKeyTextureIdx(int32 idx)&&
{
	keyTextureIdx = idx;
	return std::move(*this);
}

OptionMenuField::OptionMenuField(const TextureRegion& keyTextureRegion, const OptionMenuFieldCreateInfo& createInfo)
	: m_configIniKey(createInfo.configIniKey)
	, m_isEnum(createInfo.valueStep == 0)
	, m_suffixStr(createInfo.suffixStr)
	, m_valueDisplayNamePairs(createInfo.valueDisplayNamePairs)
	, m_menu(createInfo.valueStep == 0
		? MakeMenuEnum(static_cast<int32>(createInfo.valueDisplayNamePairs.size()), FindDefaultCursor(m_configIniKey, m_isEnum, m_valueDisplayNamePairs, 0))
		: MakeMenuInt(createInfo.valueMin, createInfo.valueMax, FindDefaultCursor(m_configIniKey, m_isEnum, m_valueDisplayNamePairs, createInfo.valueDefault), createInfo.valueStep))
	, m_keyTextureRegion(keyTextureRegion)
{
}

void OptionMenuField::update()
{
	const int32 cursorPrev = m_menu.cursor();

	m_menu.update();

	// Update config.ini value if cursor is changed
	const int32 cursor = m_menu.cursor();
	if (cursor != cursorPrev)
	{
		if (m_isEnum)
		{
			const int32 enumCount = static_cast<int32>(m_valueDisplayNamePairs.size());
			if (0 <= cursor && cursor < enumCount)
			{
				ConfigIni::SetString(m_configIniKey, m_valueDisplayNamePairs[cursor].first);
			}
			else
			{
				Print << U"Warning[ OptionMenuField::update() ]: Option enum value index is out of range! (func=OptionMenuField::draw(), index={}, min=0, max={})"_fmt(cursor, enumCount - 1);
			}
		}
		else
		{
			ConfigIni::SetInt(m_configIniKey, cursor);
		}
	}
}

void OptionMenuField::draw(const Vec2& position, const TiledTexture& valueTiledTexture, const Font& font) const
{
	using namespace ScreenUtils;

	// Draw left half (key)
	m_keyTextureRegion.draw(position);

	// Draw right half (value)
	const Vec2 rightHalfOffsetPosition{ position.x + m_keyTextureRegion.size.x, position.y };
	const int32 cursor = m_menu.cursor();
	const ArrowType arrowType = GetMenuFieldValueArrowType(m_menu);
	valueTiledTexture(arrowType).draw(rightHalfOffsetPosition);

	// Draw text on the right half
	const Vec2 textPosition = rightHalfOffsetPosition + valueTiledTexture.scaledSize() / 2;
	if (m_isEnum)
	{
		const int32 enumCount = static_cast<int32>(m_valueDisplayNamePairs.size());
		if (cursor < 0 || enumCount <= cursor)
		{
			Print << U"Warning[ OptionMenuField::draw() ]: Option enum value index is out of range! (func=OptionMenuField::draw(), index={}, min=0, max={})"_fmt(cursor, enumCount - 1);
			return;
		}

		const StringView displayName = m_valueDisplayNamePairs[cursor].second;
		font(displayName).drawAt(textPosition);
	}
	else
	{
		font(Format(cursor) + m_suffixStr).drawAt(textPosition);
	}
}
