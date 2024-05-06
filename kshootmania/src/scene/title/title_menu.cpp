#include "title_menu.hpp"

namespace
{
	constexpr int32 kMainTexCol = 0;
	constexpr int32 kSubTexCol = 1;

	constexpr int32 kMenuItemOffsetY = 260;
	constexpr int32 kMenuItemDiffY = 25;

	constexpr double kMenuCursorPosRelaxationTimeSec = 0.03;

	double MenuCursorAlphaValue(double sec)
	{
		constexpr double baseValue = 121.0 / 256;
		constexpr double maxValue = 211.0 / 256;
		constexpr double periodSec = Math::TwoPi * 0.15;
		constexpr double secOffset = 1.0 / 0.15;
		return baseValue + (maxValue - baseValue) * Periodic::Sine0_1(periodSec, sec + secOffset);
	}

	double EaseValue(double currentValue, double targetValue, double relaxationTime)
	{
		const double blendRate = 1.0 - Max(relaxationTime - Scene::DeltaTime(), 0.0) / relaxationTime;
		return Math::Lerp(currentValue, targetValue, blendRate);
	}
}

TitleMenu::TitleMenu(TitleMenuItem defaultMenuitem)
	: m_menu(
		LinearMenu::CreateInfoWithEnumCount
		{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Vertical,
				.buttonFlags = CursorButtonFlags::kArrowOrBTAllOrLaserAll,
			},
			.enumCount = TitleMenuItem::kItemEnumCount,
			.cyclic = IsCyclicMenuYN::No,
			.defaultCursor = defaultMenuitem,
		})
{
}

void TitleMenu::update()
{
	if (!m_isAlreadySelected)
	{
		const auto prevCursor = m_menu.cursor();
		m_menu.update();
		const auto cursor = m_menu.cursor();
		if (prevCursor != cursor)
		{
			m_selectSe.play();
		}

		if (KeyConfig::Down(KeyConfig::kStart))
		{
			const auto selectedItem = m_menu.cursorAs<TitleMenuItem>();
			m_selectedMenuItemSource.requestFinish(selectedItem);
			m_isAlreadySelected = true;
		}
		else if (KeyConfig::Down(KeyConfig::kBack))
		{
			m_menu.setCursor(kExit);
		}
	}

	m_easedCursorPos = EaseValue(m_easedCursorPos, static_cast<double>(m_menu.cursor()), kMenuCursorPosRelaxationTimeSec);
}

void TitleMenu::draw() const
{
	const int32 x = Scene::Center().x;

	// メニューカーソルを描画(加算)
	{
		const ScopedColorMul2D colorMultiply(MenuCursorAlphaValue(m_stopwatch.sF()));
		const ScopedRenderStates2D additive(BlendState::Additive);
		const TextureRegion textureRegion = m_menuCursorTexture();
		textureRegion.draw(x - textureRegion.size.x / 2, Scaled(kMenuItemOffsetY) + Scaled(kMenuItemDiffY) * m_easedCursorPos);
	}

	// メニュー項目を描画
	for (int32 i = 0; i < kItemEnumCount; ++i)
	{
		const int32 y = Scaled(kMenuItemOffsetY) + Scaled(kMenuItemDiffY) * i;
		{
			// 減算テクスチャを描画
			const ScopedRenderStates2D subtractive(BlendState::Subtractive);
			const TextureRegion textureRegion = m_menuItemTexture(i, kSubTexCol);
			textureRegion.draw(x - textureRegion.size.x / 2, y);
		}
		{
			// 加算テクスチャを描画
			const ScopedRenderStates2D additive(BlendState::Additive);
			const TextureRegion textureRegion = m_menuItemTexture(i, kMainTexCol);
			textureRegion.draw(x - textureRegion.size.x / 2, y);
		}
	}
}
