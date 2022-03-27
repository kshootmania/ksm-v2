#include "select_difficulty_menu.hpp"

SelectDifficultyMenu::SelectDifficultyMenu(const SelectMenu* pSelectMenu)
	: m_menu(MenuHelper::MakeHorizontalMenu(kNumDifficulties, MenuHelper::ButtonFlags::kArrowOrLaser, IsCyclicMenu::No, 0.12, 0.12, kDifficultyIdxLight))
	, m_pSelectMenu(pSelectMenu)
{
}

void SelectDifficultyMenu::update()
{
	const int32 cursorPrev = m_menu.cursor();

	m_menu.update();

	if (!m_menu.isCursorChanged())
	{
		return;
	}

	const SelectMenuSongItemInfo* pMenuItem = dynamic_cast<SelectMenuSongItemInfo*>(m_pSelectMenu->cursorMenuItem().info.get());
	if (pMenuItem == nullptr)
	{
		// Do not move the cursor if the current item is not a song
		m_menu.setCursor(cursorPrev);
		return;
	}

	const int32 cursor = m_menu.cursor();
	assert(0 <= cursor && cursor < pMenuItem->chartInfos.size());
	if (pMenuItem->chartInfos[cursor].has_value())
	{
		// If the cursor difficulty exists, okay
		return;
	}

	// If no difficulty is found, try to change the cursor to another difficulty
	int32 newCursor = cursorPrev;
	if (m_menu.isCursorIncremented())
	{
		for (int idx = cursor + 1; idx < kNumDifficulties; ++idx)
		{
			if (pMenuItem->chartInfos[idx].has_value())
			{
				newCursor = idx;
				break;
			}
		}
	}
	else if (m_menu.isCursorDecremented())
	{
		for (int idx = cursor - 1; idx >= 0; --idx)
		{
			if (pMenuItem->chartInfos[idx].has_value())
			{
				newCursor = idx;
				break;
			}
		}
	}
	m_menu.setCursor(newCursor);
}

int32 SelectDifficultyMenu::cursor() const
{
	if (m_pSelectMenu->empty())
	{
		return -1;
	}

	const SelectMenuSongItemInfo* pMenuItem = dynamic_cast<SelectMenuSongItemInfo*>(m_pSelectMenu->cursorMenuItem().info.get());
	
	if (pMenuItem == nullptr)
	{
		return -1;
	}

	const int32 cursor = m_menu.cursor();
	assert(0 <= cursor && cursor < pMenuItem->chartInfos.size());

	const int32 altCursor = GetAlternativeCursor(cursor,
		[pMenuItem](int32 idx)
		{
			return 0 <= idx && std::cmp_less(idx, pMenuItem->chartInfos.size()) && pMenuItem->chartInfos[idx].has_value();
		});

	return altCursor;
}

int32 SelectDifficultyMenu::rawCursor() const
{
	return m_menu.cursor();
}

bool SelectDifficultyMenu::isCursorChanged() const
{
	return m_menu.isCursorChanged();
}

int32 SelectDifficultyMenu::GetAlternativeCursor(int32 rawCursor, std::function<bool(int32)> difficultyExistsFunc)
{
	// If the cursor difficulty exists, return it as is
	if (difficultyExistsFunc(rawCursor))
	{
		return rawCursor;
	}

	// Alternative cursor value
	int32 altCursor = -1;

	// First try to change the cursor to a lower difficulty
	bool found = false;
	for (int idx = rawCursor - 1; idx >= 0; --idx)
	{
		if (difficultyExistsFunc(idx))
		{
			altCursor = idx;
			found = true;
			break;
		}
	}

	// If no difficulty is found, try to change the cursor to a higher difficulty
	if (!found)
	{
		for (int idx = rawCursor + 1; idx < kNumDifficulties; ++idx)
		{
			if (difficultyExistsFunc(idx))
			{
				altCursor = idx;
				break;
			}
		}
	}

	return altCursor;
}
