#include "select_difficulty_menu.hpp"
#include "select_menu.hpp"
#include "menu_item/iselect_menu_item.hpp"

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
	, m_difficultyTexture(SelectTexture::kSongDifficulty,
		{
			.row = 2,
			.column = kNumDifficulties,
			.sourceScale = SourceScale::k1x,
			.sourceSize = { 115, 110 },
		})
	, m_levelNumberTexture(SelectTexture::kSongLevelNumber,
		{
			.row = kLevelMax,
			.sourceScale = SourceScale::kL,
			.sourceSize = { 150, 120 },
		})
	, m_cursorTexture(SelectTexture::kSongDifficultyCursor,
		{
			.row = 12,
			.sourceScale = SourceScale::kL,
			.sourceSize = { 200, 200 },
		})
	, m_stopwatch(StartImmediately::Yes)
{
}

void SelectDifficultyMenu::update()
{
	const int32 cursorPrev = m_menu.cursor();

	m_menu.update();

	const int32 deltaCursor = m_menu.deltaCursor();

	// カーソルが変更なしの場合は特に何もしなくてOK
	if (deltaCursor == 0)
	{
		return;
	}

	const ISelectMenuItem& menuItem = m_pSelectMenu->cursorMenuItem();
	if (!menuItem.difficultyMenuExists())
	{
		// 難易度が存在しない項目の場合は難易度カーソルの移動をキャンセル
		m_menu.setCursor(cursorPrev);
		return;
	}

	const int32 cursor = m_menu.cursor();
	assert(0 <= cursor && cursor < kNumDifficulties);
	if (menuItem.chartInfoPtr(cursor) != nullptr)
	{
		// カーソルの難易度が存在すればその難易度から変更なしでOK
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
}

void SelectDifficultyMenu::draw(const Vec2& shakeVec) const
{
	const ISelectMenuItem& menuItem = m_pSelectMenu->cursorMenuItem();
	if (!menuItem.difficultyMenuExists())
	{
		return;
	}

	const Vec2 baseVec = Scaled(65, 128) + LeftMarginVec() + shakeVec;

	for (int32 difficultyIdx = 0; difficultyIdx < kNumDifficulties; ++difficultyIdx)
	{
		const SelectChartInfo* pChartInfo = menuItem.chartInfoPtr(difficultyIdx);

		// 難易度項目の背景を描画
		// Note: KSMv1でテクスチャの比率とは異なる縦横比で描画されていたので、ここでもそれを再現するために大きさ指定でリサイズしている
		m_difficultyTexture(pChartInfo != nullptr ? 1 : 0, difficultyIdx).resized(ScaledL(220, 220)).draw(baseVec + ScaledL(50 + 236 * difficultyIdx, 324));

		// レベルの数字を描画
		if (pChartInfo != nullptr)
		{
			m_levelNumberTexture(Clamp(pChartInfo->level() - 1, 0, kLevelMax - 1)).draw(baseVec + ScaledL(86 + 236 * difficultyIdx, 358));
		}
	}

	// カーソルのアニメーションを描画
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

int32 SelectDifficultyMenu::deltaCursor() const
{
	return m_menu.deltaCursor();
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
