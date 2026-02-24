#include "Encoding.hpp"
#include "kson/Encoding/Encoding.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace Encoding
{
	Array<String> ReadTextFileLinesShiftJISOrUTF8BasedOnBOM(FilePathView filePath)
	{
		Array<String> lines;

		const auto utf8 = filePath.toUTF8();
		const std::filesystem::path fsPath(std::u8string_view(
			reinterpret_cast<const char8_t*>(utf8.data()), utf8.size()));
		std::ifstream ifs(fsPath, std::ios_base::binary);
		if (!ifs.good())
		{
			return lines;
		}

		// BOMの有無を確認
		bool isUTF8 = false;
		{
			char bom[3];
			ifs.read(bom, 3);
			if (ifs.gcount() == 3 &&
				static_cast<uint8_t>(bom[0]) == 0xEF &&
				static_cast<uint8_t>(bom[1]) == 0xBB &&
				static_cast<uint8_t>(bom[2]) == 0xBF)
			{
				isUTF8 = true;
				// BOMが見つかった場合、ストリーム位置は既にBOMの後
			}
			else
			{
				// BOMがない場合、またはファイルが小さい場合、ファイルの先頭に戻る
				ifs.clear();
				ifs.seekg(0, std::ios_base::beg);
			}
		}

		// 行ごとに読み込んで変換
		std::string line;
		while (std::getline(ifs, line, '\n'))
		{
			// CR(\r)を削除
			if (!line.empty() && line.back() == '\r')
			{
				line.pop_back();
			}

			// エンコーディング変換
			if (isUTF8)
			{
				// UTF-8の場合はそのまま変換
				lines.push_back(Unicode::FromUTF8(line));
			}
			else
			{
				// Shift-JISの場合はUTF-8に変換してから
				const std::string lineUTF8 = kson::Encoding::ShiftJISToUTF8(line);
				lines.push_back(Unicode::FromUTF8(lineUTF8));
			}
		}

		return lines;
	}
}
