#pragma once
#include "Input/Cursor/CursorInput.hpp"

class SelectMenu;

class SelectDifficultyMenu
{
private:
	CursorInput m_cursorInput;

	const SelectMenu* const m_pSelectMenu;

	int32 m_cursor = kDifficultyIdxLight;

public:
	explicit SelectDifficultyMenu(const SelectMenu* pSelectMenu);

	/// @brief 入力を処理して入力デルタを返す(カーソル位置は変更しない)
	[[nodiscard]]
	int32 updateAndGetInputDelta();

	[[nodiscard]]
	int32 cursor() const;

	void setCursor(int32 cursor);

	[[nodiscard]]
	int32 rawCursor() const;

	[[nodiscard]]
	static int32 GetAlternativeCursor(int32 rawCursor, std::function<bool(int32)> fnDifficultyExists);
};
