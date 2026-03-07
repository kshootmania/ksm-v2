#include "SelectDifficultyMenu.hpp"
#include "SelectMenu.hpp"
#include "MenuItem/ISelectMenuItem.hpp"

SelectDifficultyMenu::SelectDifficultyMenu(const SelectMenu* pSelectMenu)
	: m_cursorInput({
		.type = CursorInput::Type::Horizontal,
		.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		.buttonIntervalSec = 0.12,
		.buttonIntervalSecFirst = 0.12,
	})
	, m_pSelectMenu(pSelectMenu)
{
}

int32 SelectDifficultyMenu::updateAndGetInputDelta()
{
	m_cursorInput.update();
	return m_cursorInput.deltaCursor();
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

	assert(0 <= m_cursor && m_cursor < kNumDifficulties);

	const int32 altCursor = GetAlternativeCursor(m_cursor,
		[&menuItem](int32 idx)
		{
			return menuItem.chartInfoPtr(idx) != nullptr;
		});

	return altCursor;
}

void SelectDifficultyMenu::setCursor(int32 cursor)
{
	m_cursor = Clamp(cursor, 0, kNumDifficulties - 1);
}

int32 SelectDifficultyMenu::rawCursor() const
{
	return m_cursor;
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
