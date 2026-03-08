#pragma once

/// @brief フルスクリーンでも正しく動作するMessageBoxラッパー
namespace MessageBoxUtils
{
	/// @brief フルスクリーン対応のMessageBoxOK(Windowsではフルスクリーン中でもOS標準ダイアログを表示)
	MessageBoxResult ShowOK(StringView text, MessageBoxStyle style = MessageBoxStyle::Default);
}
