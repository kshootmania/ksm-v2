#include "option_menu_field.hpp"
#include "ui/menu_helper.hpp"

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

	LinearMenu MakeMenu(int32 enumCount)
	{
		return MenuHelper::MakeHorizontalMenu(enumCount, MenuHelper::ButtonFlags::kArrowOrBTOrFXOrLaser);
	}

	enum MenuFieldValueArrowType
	{
		kArrowLeft = 0,
		kArrowRight,
		kArrowLeftRight,
		kArrowNone,
	};

	MenuFieldValueArrowType GetMenuFieldValueArrowType(int32 cursorIdx, int32 enumCount)
	{
		if (enumCount <= 1)
		{
			return kArrowNone;
		}
		
		if (cursorIdx == 0)
		{
			return kArrowRight;
		}

		if (cursorIdx = enumCount - 1)
		{
			return kArrowLeft;
		}

		return kArrowLeftRight;
	}
}

OptionMenuFieldCreateInfo::OptionMenuFieldCreateInfo(StringView configIniKey, const Array<std::pair<String, String>>& valueDisplayNamePairs)
	: configIniKey(configIniKey)
	, valueDisplayNamePairs(valueDisplayNamePairs)
{
}

OptionMenuFieldCreateInfo::OptionMenuFieldCreateInfo(StringView configIniKey, const Array<std::pair<int, String>>& valueDisplayNamePairs)
	: configIniKey(configIniKey)
	, valueDisplayNamePairs(ConvertValueDisplayNamePairs(valueDisplayNamePairs))
{
}

OptionMenuFieldCreateInfo::OptionMenuFieldCreateInfo(StringView configIniKey, const Array<std::pair<double, String>>& valueDisplayNamePairs)
	: configIniKey(configIniKey)
	, valueDisplayNamePairs(ConvertValueDisplayNamePairs(valueDisplayNamePairs))
{
}

OptionMenuField::OptionMenuField(const OptionMenuFieldCreateInfo& createInfo)
	: m_configIniKey(createInfo.configIniKey)
	, m_valueDisplayNamePairs(createInfo.valueDisplayNamePairs)
	, m_menu(MakeMenu(static_cast<int32>(createInfo.valueDisplayNamePairs.size())))
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

	keyTextureRegion.resized(size).draw(position);

	const int32 cursorIdx = m_menu.cursor();
	const int32 enumCount = static_cast<int32>(m_valueDisplayNamePairs.size());
	const MenuFieldValueArrowType arrowType = GetMenuFieldValueArrowType(cursorIdx, enumCount);
	valueTextureAtlas(arrowType).resized(size).draw(position + Vec2{ size.x, 0.0 });

	if (cursorIdx < 0 || enumCount <= cursorIdx)
	{
		Print << U"Warning: Option enum value index is out of range! (func=OptionMenuField::draw(), index={}, min=0, max={})"_fmt(cursorIdx, enumCount - 1);
		return;
	}
	const StringView displayName = m_valueDisplayNamePairs[cursorIdx].second;
	font(displayName).drawAt(position + Vec2{ size.x, 0.0 } + size / 2);
}
