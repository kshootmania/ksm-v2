#pragma once

/// @brief フルスクリーンでも正しく動作するMessageBoxラッパー
namespace MessageBoxUtils
{
	/// @brief フルスクリーン対応のMessageBoxOK(Windowsではフルスクリーン中でもOS標準ダイアログを表示)
	/// @return 常にMessageBoxResult::OK
	MessageBoxResult ShowOK(StringView text, MessageBoxStyle style = MessageBoxStyle::Default);

	/// @brief フルスクリーン対応のMessageBoxYesNo(Windowsではフルスクリーン中でもOS標準ダイアログを表示)
	/// @return 「はい」ならMessageBoxResult::OK、「いいえ」ならMessageBoxResult::Cancel
	MessageBoxResult ShowYesNo(StringView text, MessageBoxStyle style = MessageBoxStyle::Default);
}
