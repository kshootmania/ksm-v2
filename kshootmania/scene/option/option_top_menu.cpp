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

OptionTopMenu::OptionTopMenu(OptionScene* pOptionScene)
	: m_pOptionScene(pOptionScene)
	, m_menu(MenuHelper::MakeVerticalMenu(this, kItemMax))
	, m_menuItemTextureAtlas(OptionTexture::kTopMenuItem, kItemMax)
	, m_stopwatch(StartImmediately::Yes)
{
}

void OptionTopMenu::update()
{
	if (!m_active)
	{
		return;
	}

	m_menu.update();
}

void OptionTopMenu::draw() const
{
	using namespace ScreenUtils;

	const int32 x = Scene::Center().x;

	// Draw menu items
	for (int32 i = 0; i < kItemMax; ++i)
	{
		const int32 y = Scaled(kMenuItemOffsetY) + Scaled(kMenuItemDiffY) * i;
		const TextureRegion textureRegion = Scaled2x(m_menuItemTextureAtlas(i));
		const double alpha = MenuCursorAlphaValue(m_stopwatch.sF(), i == m_menu.cursorIdx());
		textureRegion.draw(x - textureRegion.size.x / 2, y, ColorF{ 1.0, alpha });
	}
}

void OptionTopMenu::processMenuEvent(const MenuEvent& event)
{
	if (!m_active)
	{
		return;
	}

	switch (event.trigger)
	{
	case MenuEventTrigger::Enter:
		m_active = false;
		break;

	case MenuEventTrigger::Esc:
		m_pOptionScene->exitScene();
		break;

	default:
		break;
	}
}

bool OptionTopMenu::active() const
{
	return m_active;
}

void OptionTopMenu::activate()
{
	m_active = true;
}
