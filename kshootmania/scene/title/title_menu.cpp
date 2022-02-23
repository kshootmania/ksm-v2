#include "title_menu.hpp"
#include "title_assets.hpp"

namespace
{
	constexpr int32 kMainTexCol = 0;
	constexpr int32 kSubTexCol = 1;

	constexpr int32 kMenuItemOffsetY = 260;
	constexpr int32 kMenuItemDiffY = 25;

	constexpr double kMenuCursorPosRelaxationTimeSec = 0.03;

	constexpr double kExitFadeOutDurationSec = 0.8;

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
		const double blendRate = Max(relaxationTime - Scene::DeltaTime(), 0.0) / relaxationTime;
		return Math::Lerp(currentValue, targetValue, blendRate);
	}
}

TitleMenu::TitleMenu()
	: m_menu(MakeVerticalSimpleMenu(this, kItemMax))
	, m_menuItemTextureAtlas(TitleTexture::kMenuItem, kItemMax, 2/* <- Additive Texture & Subtractive Texture */)
	, m_menuCursorTexture(TextureAsset(TitleTexture::kMenuCursor))
	, m_stopwatch(StartImmediately::Yes)
	, m_exitStopwatch(StartImmediately::No)
{
}

void TitleMenu::update()
{
	if (m_exitStopwatch.isStarted() && m_exitStopwatch.sF() > kExitFadeOutDurationSec)
	{
		System::Exit();
		return;
	}

	m_menu.update();

	m_easedCursorPos = EaseValue(m_easedCursorPos, static_cast<double>(m_menu.cursorIdx()), kMenuCursorPosRelaxationTimeSec);
}

void TitleMenu::draw() const
{
	using namespace ScreenUtils;

	const int32 x = Scene::Center().x;

	// Draw menu cursor (additive)
	{
		const ScopedColorMul2D colorMultiply(MenuCursorAlphaValue(m_stopwatch.sF()));
		const ScopedRenderStates2D additive(BlendState::Additive);
		const TextureRegion textureRegion = ScaledL(m_menuCursorTexture);
		textureRegion.draw(x - textureRegion.size.x / 2, Scaled(kMenuItemOffsetY) + Scaled(kMenuItemDiffY) * m_easedCursorPos);
	}

	// Draw menu items
	for (int32 i = 0; i < kItemMax; ++i)
	{
		const int32 y = Scaled(kMenuItemOffsetY) + Scaled(kMenuItemDiffY) * i;
		{
			// Sub-texture (subtractive)
			const ScopedRenderStates2D subtractive(BlendState::Subtractive);
			const TextureRegion textureRegion = ScaledL(m_menuItemTextureAtlas(i, kSubTexCol));
			textureRegion.draw(x - textureRegion.size.x / 2, y);
		}
		{
			// Main texture (additive)
			const ScopedRenderStates2D additive(BlendState::Additive);
			const TextureRegion textureRegion = ScaledL(m_menuItemTextureAtlas(i, kMainTexCol));
			textureRegion.draw(x - textureRegion.size.x / 2, y);
		}
	}

	// Fadeout before exit
	if (m_exitStopwatch.isStarted())
	{
		const double alpha = m_exitStopwatch.sF() / kExitFadeOutDurationSec;
		Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, alpha });
	}
}

void TitleMenu::menuKeyPressed(const MenuEvent& event)
{
	switch (event.trigger)
	{
	case MenuEventTrigger::Enter:
		menuEnterKeyPressed(event);
		break;

	case MenuEventTrigger::Esc:
		menuEscKeyPressed(event);
		break;

	default:
		break;
	}
}

void TitleMenu::menuEnterKeyPressed(const MenuEvent& event)
{
	switch (event.menuItemIdx)
	{
	case kStart:
		break;

	case kOption:
		break;

	case kInputGate:
		break;

	case kExit:
		m_exitStopwatch.start();
		break;
	}
}

void TitleMenu::menuEscKeyPressed(const MenuEvent&)
{
	m_menu.moveCursorTo(kExit);
}
