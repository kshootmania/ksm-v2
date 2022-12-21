﻿#include "select_difficulty_menu.hpp"
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

	// カーソルが変更なしの場合は特に何もしなくてOK
	if (!m_menu.isCursorChanged())
	{
		return;
	}

	const SelectMenuSongItemInfo* pMenuItem = dynamic_cast<SelectMenuSongItemInfo*>(m_pSelectMenu->cursorMenuItem().info.get());
	if (pMenuItem == nullptr)
	{
		// 選択中の項目が曲以外の場合は難易度カーソルを動かさない(元の状態に戻す)
		m_menu.setCursor(cursorPrev);
		return;
	}

	const int32 cursor = m_menu.cursor();
	assert(0 <= cursor && cursor < pMenuItem->chartInfos.size());
	if (pMenuItem->chartInfos[cursor].has_value())
	{
		// カーソルの難易度が存在すればその難易度から変更なしでOK
		return;
	}

	// カーソルが存在しない難易度に変更された場合は、他の難易度への変更を試みる
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

		// 難易度項目の背景を描画
		// Note: KSMv1でテクスチャの比率とは異なる縦横比で描画されていたので、ここでもそれを再現するために大きさ指定でリサイズしている
		m_difficultyTexture(difficultyExists ? 1 : 0, i).resized(ScaledL(220, 220)).draw(baseVec + ScaledL(50 + 236 * i, 324));

		// レベルの数字を描画
		if (difficultyExists)
		{
			m_levelNumberTexture(Clamp(pMenuItem->chartInfos[i]->level - 1, 0, kLevelMax - 1)).draw(baseVec + ScaledL(86 + 236 * i, 358));
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

void SelectDifficultyMenu::setCursor(int32 cursor)
{
	m_menu.setCursor(cursor);
}

int32 SelectDifficultyMenu::rawCursor() const
{
	return m_menu.cursor();
}

bool SelectDifficultyMenu::isCursorChanged() const
{
	return m_menu.isCursorChanged();
}

// 選択中の曲にカーソルの難易度が存在するとは限らないので、存在する難易度のうちカーソルに最も近いものを代替カーソル値(alternative cursor)として返す
int32 SelectDifficultyMenu::GetAlternativeCursor(int32 rawCursor, std::function<bool(int32)> difficultyExistsFunc)
{
	// カーソルの難易度が存在すればそれをそのまま返す
	if (difficultyExistsFunc(rawCursor))
	{
		return rawCursor;
	}

	// 代替カーソル値
	int32 altCursor = -1;

	// はじめにカーソルより下の難易度への変更を試みる
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

	// もし見つからなければ、カーソルより上の難易度への変更を試みる
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