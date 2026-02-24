#include "IRHash.hpp"
#include <Siv3D/MD5.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "kson/IO/KsonIO.hpp"
#include "kson/IO/KshIO.hpp"

namespace
{
	// IRハッシュ計算時に除外する行のprefix一覧
	constexpr std::string_view kIgnorePrefixes[] = {
		"//",
		"title=",
		"title_img=",
		"artist=",
		"artist_img=",
		"effect=",
		"illustrator=",
		"jacket=",
		"level=",
		"difficulty=",
		"m=",
		"o=",
		"v=",
		"vo=",
		"mvol=",
		"bg=",
		"layer=",
		"po=",
		"plength=",
		"pfiltergain=",
		"filtertype=",
		"chokkakuautovol=",
		"chokkakuvol=",
		"chokkakuse=",
		"icon=",
		"ver=",
		"fx-l=",
		"fx-r=",
		"#define",
		"information=",
		"ver_compat=",
		"title_translit=",
		"artist_translit=",
	};

	bool ShouldIgnoreLine(std::string_view line)
	{
		for (const auto& prefix : kIgnorePrefixes)
		{
			if (line.starts_with(prefix))
			{
				return true;
			}
		}
		return false;
	}

	// KSH形式のテキストからIRハッシュを計算
	String CalculateFromKshContent(std::string_view content)
	{
		// 末尾の'\n'を除去
		if (!content.empty() && content.back() == '\n')
		{
			content = content.substr(0, content.size() - 1);
		}

		std::string filtered;
		filtered.reserve(content.size());

		std::string_view remaining = content;
		while (!remaining.empty())
		{
			// 次の改行を探す
			const auto newlinePos = remaining.find('\n');
			std::string_view line;
			if (newlinePos != std::string_view::npos)
			{
				line = remaining.substr(0, newlinePos);
				remaining = remaining.substr(newlinePos + 1);
			}
			else
			{
				line = remaining;
				remaining = {};
			}

			// 除外対象でない行のみ追加
			if (!ShouldIgnoreLine(line))
			{
				filtered += line;
				filtered += '\n';
			}
		}

		if (filtered.empty())
		{
			return {};
		}

		const MD5Value md5 = MD5::FromBinary(filtered.data(), filtered.size());
		return md5.asString();
	}

	// KSHファイルからIRハッシュを計算
	String CalculateFromKshFile(FilePathView chartFilePath)
	{
		const auto utf8 = chartFilePath.toUTF8();
		const std::filesystem::path fsPath(std::u8string_view(
			reinterpret_cast<const char8_t*>(utf8.data()), utf8.size()));
		std::ifstream ifs(fsPath, std::ios_base::binary);
		if (!ifs.good())
		{
			return {};
		}

		// ファイル全体を読み込む
		std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

		// UTF-8 BOMをスキップ
		std::string_view contentView = content;
		if (contentView.size() >= 3 &&
			static_cast<uint8_t>(contentView[0]) == 0xEF &&
			static_cast<uint8_t>(contentView[1]) == 0xBB &&
			static_cast<uint8_t>(contentView[2]) == 0xBF)
		{
			contentView = contentView.substr(3);
		}

		return CalculateFromKshContent(contentView);
	}

	// KSONファイルからIRハッシュを計算(KSH形式に変換してから処理)
	String CalculateFromKsonFile(FilePathView chartFilePath)
	{
		// KSON読み込み
		const kson::ChartData chartData = kson::LoadKsonChartData(chartFilePath.toUTF8());
		if (chartData.error != kson::ErrorType::None)
		{
			return {};
		}

		// KSH形式に変換
		std::ostringstream oss;
		const kson::ErrorType saveError = kson::SaveKshChartData(oss, chartData);
		if (saveError != kson::ErrorType::None)
		{
			return {};
		}

		const std::string kshContent = oss.str();

		// BOMをスキップ
		std::string_view contentView = kshContent;
		if (contentView.size() >= 3 &&
			static_cast<uint8_t>(contentView[0]) == 0xEF &&
			static_cast<uint8_t>(contentView[1]) == 0xBB &&
			static_cast<uint8_t>(contentView[2]) == 0xBF)
		{
			contentView = contentView.substr(3);
		}

		return CalculateFromKshContent(contentView);
	}
}

String IRHash::Calculate(FilePathView chartFilePath)
{
	if (FsUtils::HasKsonExtension(chartFilePath))
	{
		return CalculateFromKsonFile(chartFilePath);
	}
	else
	{
		return CalculateFromKshFile(chartFilePath);
	}
}
