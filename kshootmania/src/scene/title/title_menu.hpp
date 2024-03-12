#pragma once
#include <CoTaskLib.hpp>
#include "title_assets.hpp"
#include "graphics/tiled_texture.hpp"
#include "ksmaudio/ksmaudio.hpp"

enum TitleMenuItem : int32
{
	kStart = 0,
	kOption,
	kInputGate,
	kExit,

	kItemEnumCount,
};

class TitleMenu
{
private:
	Optional<TitleMenuItem> m_selectedItem = none;

	LinearMenu m_menu
	{
		LinearMenu::CreateInfoWithEnumCount
		{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Vertical,
				.buttonFlags = CursorButtonFlags::kArrowOrBTAllOrLaserAll,
			},
			.enumCount = TitleMenuItem::kItemEnumCount,
			.cyclic = IsCyclicMenuYN::No,
		}
	};

	TiledTexture m_menuItemTexture
	{
		TitleTexture::kMenuItem,
		{
			.row = kItemEnumCount,
			.column = 2, // 加算テクスチャと減算テクスチャの2枚
			.sourceScale = SourceScale::k3x,
			.sourceSize = { 525, 75 },
		}
	};

	TiledTexture m_menuCursorTexture{ TitleTexture::kMenuCursor, SourceScale::k3x };

	Stopwatch m_stopwatch { StartImmediately::Yes };

	double m_easedCursorPos = 0.0;

	ksmaudio::Sample m_selectSe{ "se/title_sel.wav" };

	void update();

public:
	TitleMenu() = default;

	TitleMenu(TitleMenu&&) = default;

	TitleMenu& operator=(TitleMenu&&) = default;

	CoTask<TitleMenuItem> waitForSelection();

	void draw() const;
};
