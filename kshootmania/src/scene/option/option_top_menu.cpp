#include "option_top_menu.hpp"
#include "option_scene.hpp"
#include "option_assets.hpp"

namespace
{
	constexpr int32 kMenuItemOffsetY = 40;
	constexpr int32 kMenuItemDiffY = 100;

	double MenuCursorAlphaValue(double sec, bool isSelected)
	{
		if (!isSelected)
		{
			return 100.0 / 256;
		}

		constexpr double baseValue = 184.0 / 256;
		constexpr double maxValue = 280.0 / 256;
		constexpr double periodSec = Math::TwoPi * 0.15;
		constexpr double secOffset = 1.0 / 0.15;
		return Min(baseValue + (maxValue - baseValue) * Periodic::Sine0_1(periodSec, sec + secOffset), 1.0);
	}
}

OptionTopMenu::OptionTopMenu()
	: m_menu(
		LinearMenu::CreateInfoWithEnumCount{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Vertical,
				.buttonFlags = CursorButtonFlags::kArrowOrBTOrLaser,
				.buttonIntervalSec = 0.1,
				.buttonIntervalSecFirst = 0.5,
			},
			.enumCount = kItemEnumCount,
		})
	, m_itemTiledTexture(OptionTexture::kTopMenuItem,
		{
			.row = kItemEnumCount,
			.sourceScale = SourceScale::k2x,
			.sourceSize = { 960, 160 },
		})
	, m_stopwatch(StartImmediately::Yes)
{
}

void OptionTopMenu::update()
{
	m_menu.update();
}

void OptionTopMenu::draw() const
{
	const int32 x = Scene::Center().x;

	// Draw menu items
	const int32 cursorIdx = m_menu.cursor();
	for (int32 i = 0; i < kItemEnumCount; ++i)
	{
		const int32 y = Scaled(kMenuItemOffsetY) + Scaled(kMenuItemDiffY) * i;
		const TextureRegion textureRegion = m_itemTiledTexture(i);
		const double alpha = MenuCursorAlphaValue(m_stopwatch.sF(), i == cursorIdx);
		textureRegion.draw(x - textureRegion.size.x / 2, y, ColorF{ 1.0, alpha });
	}
}
