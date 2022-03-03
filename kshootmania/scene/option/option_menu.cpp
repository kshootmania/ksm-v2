#include "option_menu.hpp"
#include "option_assets.hpp"
#include "ui/menu_helper.hpp"

namespace
{
	constexpr int32 kFieldDiffY = 30;
	constexpr int32 kFieldWidth = 600;
	constexpr int32 kFieldHeight = 24;

	Array<OptionMenuField> MakeFields(const Array<OptionMenuFieldCreateInfo>& fieldCreateInfos)
	{
		return fieldCreateInfos.map([](const OptionMenuFieldCreateInfo& createInfo) {
			return OptionMenuField(createInfo);
		});
	}

	double MenuCursorAlphaValue(double sec)
	{
		constexpr double baseValue = 52.0 / 256;
		constexpr double maxValue = 188.0 / 256;
		constexpr double periodSec = Math::TwoPi * 0.15;
		constexpr double secOffset = 1.0 / 0.15;
		return Min(baseValue + (maxValue - baseValue) * Periodic::Sine0_1(periodSec, sec + secOffset), 1.0);
	}
}

OptionMenu::OptionMenu(StringView fieldKeyTextureAssetKey, const Array<OptionMenuFieldCreateInfo>& fieldCreateInfos)
	: m_menu(MenuHelper::MakeVerticalMenu(static_cast<int32>(fieldCreateInfos.size()), MenuHelper::ButtonFlags::kArrowOrBTOrLaser))
	, m_fields(MakeFields(fieldCreateInfos))
	, m_fieldKeyTextureAtlas(fieldKeyTextureAssetKey, static_cast<int32>(fieldCreateInfos.size()))
	, m_fieldValueTextureAtlas(OptionTexture::kMenuFieldValue, 4)
	, m_fieldCursorTexture(TextureAsset(OptionTexture::kMenuFieldCursor))
	, m_font(ScreenUtils::Scaled(15), FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default)
	, m_stopwatch(StartImmediately::Yes)
{
}

void OptionMenu::update()
{
	m_menu.update();

	if (!m_fields.empty())
	{
		const int32 cursorIdx = m_menu.cursor();
		const int32 enumCount = static_cast<int32>(m_fields.size());
		if (cursorIdx < 0 || enumCount <= cursorIdx)
		{
			Print << U"Warning: Menu cursor is out of range! (func=OptionMenu::update(), cursor={}, min=0, max={})"_fmt(cursorIdx, enumCount - 1);
			return;
		}
		m_fields[cursorIdx].update();
	}
}

void OptionMenu::draw(const Vec2& position) const
{
	using namespace ScreenUtils;

	for (int32 i = 0; i < static_cast<int32>(m_fields.size()); ++i)
	{
		const Vec2 fieldPos = position + Vec2{ 0, Scaled(kFieldDiffY) * i };

		m_fields[i].draw(fieldPos, m_fieldKeyTextureAtlas(i), m_fieldValueTextureAtlas, m_font);

		// Draw cursor (additive)
		if (i == m_menu.cursor())
		{
			const ScopedRenderStates2D additive(BlendState::Additive);
			const double alpha = MenuCursorAlphaValue(m_stopwatch.sF());
			m_fieldCursorTexture.resized(Scaled(kFieldWidth), Scaled(kFieldHeight)).draw(fieldPos, ColorF{ 1.0, alpha });
		}
	}
}
