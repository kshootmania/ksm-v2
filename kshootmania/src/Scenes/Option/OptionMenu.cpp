#include "OptionMenu.hpp"

namespace
{
	Array<OptionMenuField> MakeFields(const Array<OptionMenuField::CreateInfo>& fieldCreateInfos)
	{
		return fieldCreateInfos.map([](const OptionMenuField::CreateInfo& createInfo) {
			return OptionMenuField(createInfo);
		});
	}
}

OptionMenu::OptionMenu(const Array<OptionMenuField::CreateInfo>& fieldCreateInfos)
	: m_menu(
		LinearMenu::CreateInfoWithEnumCount{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Vertical,
				.buttonFlags = CursorButtonFlags::kArrowOrLaser | CursorButtonFlags::kMouseWheel,
				.buttonIntervalSec = 0.1,
				.buttonIntervalSecFirst = 0.5,
			},
			.enumCount = static_cast<int32>(fieldCreateInfos.size()),
		})
	, m_fields(MakeFields(fieldCreateInfos))
{
	KeyConfig::SaveToConfigIni();
}

bool OptionMenu::update()
{
	return update(MouseInput{});
}

bool OptionMenu::update(const MouseInput& mouseInput)
{
	m_menu.update();

	bool cursorChanged = m_menu.deltaCursor() != 0;

	// クリックされた項目へカーソルを移動
	if (mouseInput.clickedItemIdx.has_value() && *mouseInput.clickedItemIdx != m_menu.cursor())
	{
		m_menu.setCursor(*mouseInput.clickedItemIdx);
		cursorChanged = true;
	}

	bool valueChanged = false;

	if (!m_fields.empty())
	{
		const int32 cursorIdx = m_menu.cursor();
		const int32 enumCount = static_cast<int32>(m_fields.size());
		if (cursorIdx < 0 || enumCount <= cursorIdx)
		{
			Logger << U"[ksm warning] OptionMenu::update(): Menu cursor is out of range! (func=OptionMenu::update(), cursor={}, min=0, max={})"_fmt(cursorIdx, enumCount - 1);
			return false;
		}
		valueChanged = m_fields[cursorIdx].update(mouseInput.valueDelta);
	}

	return cursorChanged || valueChanged;
}

Array<OptionMenu::SettingItemParams> OptionMenu::getSettingItemParamsList() const
{
	Array<SettingItemParams> result;
	result.reserve(m_fields.size());

	const int32 cursorIdx = m_menu.cursor();
	for (int32 i = 0; i < static_cast<int32>(m_fields.size()); ++i)
	{
		const OptionMenuField::ArrowType arrowType = m_fields[i].arrowType();
		result.push_back({
			.name = m_fields[i].name(),
			.value = m_fields[i].valueDisplayString(),
			.selected = (i == cursorIdx),
			.valueArrowIndex = static_cast<int32>(arrowType),
			.valueLeftArrowVisible = arrowType == OptionMenuField::kArrowTypeLeft || arrowType == OptionMenuField::kArrowTypeLeftRight,
			.valueRightArrowVisible = arrowType == OptionMenuField::kArrowTypeRight || arrowType == OptionMenuField::kArrowTypeLeftRight,
		});
	}

	return result;
}

int32 OptionMenu::cursor() const
{
	return m_menu.cursor();
}

void OptionMenu::setCursor(int32 value)
{
	m_menu.setCursor(value);
}
