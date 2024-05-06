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
	bool m_isAlreadySelected = false;

	LinearMenu m_menu;

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

	Co::TaskFinishSource<TitleMenuItem> m_selectedMenuItemSource;

public:
	TitleMenu(TitleMenuItem defaultMenuitem);

	TitleMenu(TitleMenu&&) = default;

	TitleMenu& operator=(TitleMenu&&) = default;

	void update();

	void draw() const;

	Co::Task<TitleMenuItem> selectedMenuItemAsync() const
	{
		return m_selectedMenuItemSource.waitForFinish();
	}
};
