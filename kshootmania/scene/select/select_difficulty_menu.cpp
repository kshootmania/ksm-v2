#include "select_difficulty_menu.hpp"
#include "select_menu.hpp"

SelectDifficultyMenu::SelectDifficultyMenu(const SelectMenu* pSelectMenu)
	: m_menu(MenuHelper::MakeHorizontalMenu(kNumDifficulties, MenuHelper::ButtonFlags::kArrowOrLaser, IsCyclicMenu::No, 0.12, 0.12, kDifficultyIdxLight))
	, m_pSelectMenu(pSelectMenu)
	, m_difficultyTexture(SelectTexture::kSongDifficulty,
		{
			.row = 2,
			.column = kNumDifficulties,
			.sourceScale = ScreenUtils::SourceScale::k1x,
			.sourceSize = { 115, 110 },
		})
	, m_levelNumberTexture(SelectTexture::kSongLevelNumber,
		{
			.row = kLevelMax,
			.sourceScale = ScreenUtils::SourceScale::kL,
			.sourceSize = { 150, 120 },
		})
	, m_cursorTexture(SelectTexture::kSongDifficultyCursor,
		{
			.row = 12,
			.sourceScale = ScreenUtils::SourceScale::kL,
			.sourceSize = { 200, 200 },
		})
	, m_stopwatch(StartImmediately::Yes)
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
		// If the cursor difficulty exists, it is okay
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

void SelectDifficultyMenu::draw(const Vec2& shakeVec) const
{
	using namespace ScreenUtils;

	const SelectMenuSongItemInfo* pMenuItem = dynamic_cast<SelectMenuSongItemInfo*>(m_pSelectMenu->cursorMenuItem().info.get());

	if (pMenuItem == nullptr)
	{
		return;
	}

	const Vec2 baseVec = Scaled(65, 128) + LeftMarginVec() + shakeVec;

	for (int32 i = 0; i < kNumDifficulties; ++i)
	{
		const bool difficultyExists = std::cmp_less(i, pMenuItem->chartInfos.size()) && pMenuItem->chartInfos[i].has_value();

		// Draw difficulty item BG
		// Note: Here, the incorrect texture aspect ratio in KSMv1 is simulated
		m_difficultyTexture(difficultyExists ? 1 : 0, i).resized(ScaledL(220, 220)).draw(baseVec + ScaledL(50 + 236 * i, 324));

		// Draw level number
		if (difficultyExists)
		{
			m_levelNumberTexture(Clamp(pMenuItem->chartInfos[i]->level, 0, kLevelMax - 1)).draw(baseVec + ScaledL(86 + 236 * i, 358));
		}
	}

	// Draw cursor animation
	{
		const ScopedRenderStates2D renderState(BlendState::Additive);
		const Vec2 position = Scaled(65 - 13, 125) + ScaledL(236 * cursor(), 246) + LeftMarginVec() + shakeVec;
		m_cursorTexture(static_cast<int32>(m_stopwatch.sF() / 0.07) % 12).scaled(2.0).draw(position);
	}
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
