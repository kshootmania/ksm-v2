#include "SelectDifficultyMenu.hpp"
#include "SelectMenu.hpp"
#include "MenuItem/ISelectMenuItem.hpp"

SelectDifficultyMenu::SelectDifficultyMenu(const SelectMenu* pSelectMenu)
	: m_menu(
		LinearMenu::CreateInfoWithEnumCount{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Horizontal,
				.buttonFlags = CursorButtonFlags::kArrowOrLaser,
				.buttonIntervalSec = 0.12,
				.buttonIntervalSecFirst = 0.12,
			},
			.enumCount = kNumDifficulties,
			.defaultCursor = kDifficultyIdxLight,
		})
	, m_pSelectMenu(pSelectMenu)
	, m_stopwatch(StartImmediately::Yes)
{
}

void SelectDifficultyMenu::update(bool* pCursorChanged)
{
	const int32 cursorPrev = m_menu.cursor();

	m_menu.update();

	const int32 deltaCursor = m_menu.deltaCursor();

	// カーソルが変更なしの場合は特に何もしなくてOK
	if (deltaCursor == 0)
	{
		if (pCursorChanged != nullptr)
		{
			*pCursorChanged = false;
		}
		return;
	}

	// メニューが空の場合は何もしない
	if (m_pSelectMenu->empty())
	{
		m_menu.setCursor(cursorPrev);
		if (pCursorChanged != nullptr)
		{
			*pCursorChanged = false;
		}
		return;
	}

	const ISelectMenuItem& menuItem = m_pSelectMenu->cursorMenuItem();
	if (!menuItem.difficultyMenuExists())
	{
		// 難易度が存在しない項目の場合は難易度カーソルの移動をキャンセル
		m_menu.setCursor(cursorPrev);
		if (pCursorChanged != nullptr)
		{
			*pCursorChanged = false;
		}
		return;
	}

	const int32 cursor = m_menu.cursor();
	assert(0 <= cursor && cursor < kNumDifficulties);
	if (menuItem.chartInfoPtr(cursor) != nullptr)
	{
		// カーソルの難易度が存在すればその難易度から変更なしでOK
		if (pCursorChanged != nullptr)
		{
			*pCursorChanged = cursor != cursorPrev;
		}
		return;
	}

	// カーソルが存在しない難易度に変更された場合は、他の難易度への変更を試みる
	int32 newCursor = cursorPrev;
	if (deltaCursor > 0)
	{
		for (int idx = cursor + 1; idx < kNumDifficulties; ++idx)
		{
			if (menuItem.chartInfoPtr(idx) != nullptr)
			{
				newCursor = idx;
				break;
			}
		}
	}
	else if (deltaCursor < 0)
	{
		for (int idx = cursor - 1; idx >= 0; --idx)
		{
			if (menuItem.chartInfoPtr(idx) != nullptr)
			{
				newCursor = idx;
				break;
			}
		}
	}

	m_menu.setCursor(newCursor);

	if (pCursorChanged != nullptr)
	{
		*pCursorChanged = newCursor != cursorPrev;
	}
}

int32 SelectDifficultyMenu::cursor() const
{
	if (m_pSelectMenu->empty())
	{
		return -1;
	}

	const ISelectMenuItem& menuItem = m_pSelectMenu->cursorMenuItem();
	if (!menuItem.difficultyMenuExists())
	{
		return -1;
	}

	const int32 cursor = m_menu.cursor();
	assert(0 <= cursor && cursor < kNumDifficulties);

	const int32 altCursor = GetAlternativeCursor(cursor,
		[&menuItem](int32 idx)
		{
			return menuItem.chartInfoPtr(idx) != nullptr;
		});

	return altCursor;
}

void SelectDifficultyMenu::setCursor(int32 cursor)
{
	m_menu.setCursor(cursor);
}

int32 SelectDifficultyMenu::rawCursor() const
{
	return m_menu.cursor();
}

// 選択中の曲にカーソルの難易度が存在するとは限らないので、存在する難易度のうちカーソルに最も近いものを代替カーソル値(alternative cursor)として返す
int32 SelectDifficultyMenu::GetAlternativeCursor(int32 rawCursor, std::function<bool(int32)> fnDifficultyExists)
{
	// カーソルの難易度が存在すればそれをそのまま返す
	if (fnDifficultyExists(rawCursor))
	{
		return rawCursor;
	}

	// 代替カーソル値
	int32 altCursor = -1;

	// はじめにカーソルより下の難易度への変更を試みる
	bool found = false;
	for (int idx = rawCursor - 1; idx >= 0; --idx)
	{
		if (fnDifficultyExists(idx))
		{
			altCursor = idx;
			found = true;
			break;
		}
	}

	// もし見つからなければ、カーソルより上の難易度への変更を試みる
	if (!found)
	{
		for (int idx = rawCursor + 1; idx < kNumDifficulties; ++idx)
		{
			if (fnDifficultyExists(idx))
			{
				altCursor = idx;
				break;
			}
		}
	}

	return altCursor;
}
