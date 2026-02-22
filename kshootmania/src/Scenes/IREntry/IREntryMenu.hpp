#pragma once
#include <CoTaskLib.hpp>
#include "UI/LinearMenu.hpp"

class IREntryMenu
{
public:
	enum MenuItem : int32
	{
		kSignIn = 0,
		kDontUse,

		kItemEnumCount,
	};

private:
	bool m_isAlreadySelected = false;

	LinearMenu m_menu;

	std::shared_ptr<noco::Canvas> m_canvas;

	Stopwatch m_stopwatch{ StartImmediately::Yes };

	ksmaudio::Sample m_selectSe{ "se/title_sel.wav" };

	Co::TaskFinishSource<Optional<MenuItem>> m_selectedMenuItemSource;

	void refreshCanvasCursorIndex();

public:
	explicit IREntryMenu(const std::shared_ptr<noco::Canvas>& canvas);

	void update();

	Co::Task<Optional<MenuItem>> selectedMenuItemAsync()
	{
		return m_selectedMenuItemSource.waitForResult();
	}
};
