#pragma once

namespace MouseMenuUtils
{
	/// @brief Clickイベントが発火したタグに対応するカーソル値を返す
	/// @param canvas 対象のCanvas
	/// @param tagToCursorPairs タグとカーソル値の対応関係
	/// @return 発火したタグに対応するカーソル値(発火していない場合はnone)
	[[nodiscard]]
	Optional<int32> FiredClickIndex(const noco::Canvas& canvas, const Array<std::pair<String, int32>>& tagToCursorPairs);
}
