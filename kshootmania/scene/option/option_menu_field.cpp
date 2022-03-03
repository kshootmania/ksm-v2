#include "option_menu_field.hpp"
#include "ui/menu_helper.hpp"
#include "ini/config_ini.hpp"

namespace
{
	constexpr int32 kHalfWidth = 300;
	constexpr int32 kHeight = 24;

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

	// Note: The order is strange, but do not change this in order to preserve the order of the textures.
	enum MenuFieldValueArrowType
	{
		kArrowLeft = 0,
		kArrowRight,
		kArrowLeftRight,
		kArrowNone,
	};

	MenuFieldValueArrowType GetMenuFieldValueArrowType(const LinearMenu& menu)
	{
		const bool isMin = menu.isCursorMin();
		const bool isMax = menu.isCursorMax();

		if (isMin && isMax)
		{
			return kArrowNone;
		}

		if (isMin)
		{
			return kArrowRight;
		}

		if (isMax)
		{
			return kArrowLeft;
		}

		return kArrowLeftRight;
	}

	int32 FindDefaultCursor(StringView configIniKey, bool isEnum, const Array<std::pair<String, String>>& valueDisplayNamePairs, int32 cursorMin)
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
				return ConfigIni::GetInt(configIniKey, cursorMin);
			}
		}

		return cursorMin;
	}
}

OptionMenuFieldCreateInfo::OptionMenuFieldCreateInfo(StringView configIniKey, const Array<String>& valueDisplayNames)
	: OptionMenuFieldCreateInfo(configIniKey, ConvertValueDisplayNamePairs(valueDisplayNames))
{
}

OptionMenuFieldCreateInfo::OptionMenuFieldCreateInfo(StringView configIniKey, const Array<StringView>& valueDisplayNames)
	: OptionMenuFieldCreateInfo(configIniKey, ConvertValueDisplayNamePairs(valueDisplayNames))
{
}

OptionMenuFieldCreateInfo::OptionMenuFieldCreateInfo(StringView configIniKey, const Array<std::pair<String, String>>& valueDisplayNamePairs)
	: configIniKey(configIniKey)
	, valueDisplayNamePairs(valueDisplayNamePairs)
{
}

OptionMenuFieldCreateInfo::OptionMenuFieldCreateInfo(StringView configIniKey, const Array<std::pair<int, String>>& valueDisplayNamePairs)
	: OptionMenuFieldCreateInfo(configIniKey, ConvertValueDisplayNamePairs(valueDisplayNamePairs))
{
}

OptionMenuFieldCreateInfo::OptionMenuFieldCreateInfo(StringView configIniKey, const Array<std::pair<double, String>>& valueDisplayNamePairs)
	: OptionMenuFieldCreateInfo(configIniKey, ConvertValueDisplayNamePairs(valueDisplayNamePairs))
{
}

OptionMenuFieldCreateInfo::OptionMenuFieldCreateInfo(StringView configIniKey, int32 valueMin, int32 valueMax, StringView suffixStr, int32 valueStep)
	: configIniKey(configIniKey)
	, valueMin(valueMin)
	, valueMax(valueMax)
	, valueStep(valueStep)
	, suffixStr(suffixStr)
{
}

OptionMenuField::OptionMenuField(const OptionMenuFieldCreateInfo& createInfo)
	: m_configIniKey(createInfo.configIniKey)
	, m_isEnum(createInfo.valueStep == 0)
	, m_suffixStr(createInfo.suffixStr)
	, m_valueDisplayNamePairs(createInfo.valueDisplayNamePairs)
	, m_menu(createInfo.valueStep == 0
		? MakeMenuEnum(static_cast<int32>(createInfo.valueDisplayNamePairs.size()), FindDefaultCursor(m_configIniKey, m_isEnum, m_valueDisplayNamePairs, createInfo.valueMin))
		: MakeMenuInt(createInfo.valueMin, createInfo.valueMax, FindDefaultCursor(m_configIniKey, m_isEnum, m_valueDisplayNamePairs, createInfo.valueMin), createInfo.valueStep))
{
}

void OptionMenuField::update()
{
	m_menu.update();
}

void OptionMenuField::draw(const Vec2& position, const TextureRegion& keyTextureRegion, const TextureAtlas& valueTextureAtlas, const Font& font) const
{
	using namespace ScreenUtils;

	const Vec2 size{ Scaled(kHalfWidth), Scaled(kHeight) };

	// Draw left half (key)
	keyTextureRegion.resized(size).draw(position);

	// Draw right half (value)
	const int32 cursor = m_menu.cursor();
	const int32 enumCount = static_cast<int32>(m_valueDisplayNamePairs.size());
	const MenuFieldValueArrowType arrowType = GetMenuFieldValueArrowType(m_menu);
	valueTextureAtlas(arrowType).resized(size).draw(position + Vec2{ size.x, 0.0 });

	// Draw text on the right half
	const Vec2 textPosition = position + Vec2{ size.x, 0.0 } + size / 2;
	if (m_isEnum)
	{
		if (cursor < 0 || enumCount <= cursor)
		{
			Print << U"Warning: Option enum value index is out of range! (func=OptionMenuField::draw(), index={}, min=0, max={})"_fmt(cursor, enumCount - 1);
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
