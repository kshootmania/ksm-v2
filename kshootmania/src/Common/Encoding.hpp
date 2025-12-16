#pragma once
#include <Siv3D.hpp>

namespace Encoding
{
	// テキストファイルをUTF-8 BOMの有無に基づいて行ごとに読み込む
	// (BOMがあればUTF-8、なければShift-JISとして読み込んで変換)
	[[nodiscard]]
	Array<String> ReadTextFileLinesShiftJISOrUTF8BasedOnBOM(FilePathView filePath);
}
