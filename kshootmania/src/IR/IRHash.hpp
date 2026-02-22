#pragma once

namespace IRHash
{
	/// @brief 譜面ファイルからIRハッシュ(MD5文字列)を計算
	/// @param chartFilePath 譜面ファイルのパス(.kshまたは.kson)
	/// @return IRハッシュ文字列(計算失敗時は空文字列)
	[[nodiscard]]
	String Calculate(FilePathView chartFilePath);
}
