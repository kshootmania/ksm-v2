#include "ksc_io.hpp"

namespace KscIo
{
	namespace
	{
		bool TryConvertChartFilePathToKscPath(FilePathView chartFilePath, FilePath* pFilePath)
		{
			if (pFilePath == nullptr)
			{
				assert(false && "pFilePath must not be NULL");
				return false;
			}

			// TODO: ksonが増えるまでに決め打ちをどうにかする
			const auto relativeChartFilePath = FileSystem::RelativePath(chartFilePath, FileSystem::FullPath(U"songs"));
			if (relativeChartFilePath.size() < 4U || String{ relativeChartFilePath.substr(relativeChartFilePath.size() - 4U) }.lowercased() != U".ksh")
			{
				return false;
			}
			*pFilePath = U"score/PLAYER/{}.ksc"_fmt(relativeChartFilePath.substr(0, relativeChartFilePath.size() - 4U));
			return true;
		}

		bool TryReadHighScoreInfo(FilePathView kscFilePath, const KscKey& condition, HighScoreInfo* pHighScoreInfo)
		{
			if (pHighScoreInfo == nullptr)
			{
				assert(false && "pHighScoreInfo must not be NULL");
				return false;
			}

			TextReader reader(kscFilePath);
			if (!reader)
			{
				return false;
			}

			const String searchEasy = condition.withGaugeType(GaugeType::kEasyGauge).toString() + U"=";
			const String searchNormal = condition.withGaugeType(GaugeType::kNormalGauge).toString() + U"=";
			const String searchHard = condition.withGaugeType(GaugeType::kHardGauge).toString() + U"=";

			String line;
			while (reader.readLine(line))
			{
				if (line.starts_with(searchEasy))
				{
					const String valueStr = line.substr(searchEasy.length());
					pHighScoreInfo->easyGauge = KscValue::FromString(valueStr);
				}

				if (line.starts_with(searchNormal))
				{
					const String valueStr = line.substr(searchNormal.length());
					pHighScoreInfo->normalGauge = KscValue::FromString(valueStr);
				}

				if (line.starts_with(searchHard))
				{
					const String valueStr = line.substr(searchHard.length());
					pHighScoreInfo->hardGauge = KscValue::FromString(valueStr);
				}
			}
			return true;
		}

		bool CreateNewKscFile(FilePathView kscFilePath, const MusicGame::PlayResult& playResult, const KscKey& condition)
		{
			TextWriter writer(kscFilePath, TextEncoding::UTF8_NO_BOM);
			if (!writer)
			{
				return false;
			}
			const KscValue kscValue = KscValue{}.applyPlayResult(playResult);
			writer.write(U"{}={}\n"_fmt(condition.toString(), kscValue.toString()));
			return true;
		}

		bool UpdateExistingKscFile(FilePathView kscFilePath, const MusicGame::PlayResult& playResult, const KscKey& condition)
		{
			assert(playResult.playOption.gaugeType == condition.gaugeType && "Gauge type mismatch");
			const GaugeType gaugeType = playResult.playOption.gaugeType;

			// 既存のkscファイルを読み込んで、今回のプレイ結果を反映したkscファイルの文字列を生成
			String newKscFileContent;
			{
				const String search = condition.toString() + U"=";
				HighScoreInfo origHighScoreInfo;
				if (!TryReadHighScoreInfo(kscFilePath, condition, &origHighScoreInfo))
				{
					return false;
				}
				const KscValue& origKscValue = origHighScoreInfo.kscValueOf(gaugeType);
				const String replace = search + origKscValue.applyPlayResult(playResult).toString();

				TextReader reader(kscFilePath);
				if (!reader)
				{
					return false;
				}

				bool found = false;
				String line;
				while (reader.readLine(line))
				{
					if (line.empty())
					{
						// 空行は除去
						continue;
					}

					// 対象の行を新しいハイスコア情報で置換
					if (line.starts_with(search))
					{
						newKscFileContent += replace + U"\n";
						found = true;
					}
					else
					{
						newKscFileContent += line + U"\n";
					}
				}

				if (!found)
				{
					// 対象の行が見つからなかった場合は新しい行にハイスコア情報を追記
					newKscFileContent += replace + U"\n";
				}
			}

			// 既存のkscファイルを上書き
			{
				TextWriter writer(kscFilePath, TextEncoding::UTF8_NO_BOM);
				if (!writer)
				{
					return false;
				}
				writer.write(newKscFileContent);
			}

			return true;
		}
	}

	HighScoreInfo ReadHighScoreInfo(FilePathView chartFilePath, const KscKey& condition)
	{
		FilePath kscFilePath;
		if (!TryConvertChartFilePathToKscPath(chartFilePath, &kscFilePath))
		{
			return HighScoreInfo{};
		}

		HighScoreInfo highScoreInfo;
		if (!TryReadHighScoreInfo(kscFilePath, condition, &highScoreInfo))
		{
			return HighScoreInfo{};
		}

		return highScoreInfo;
	}

	bool WriteHighScoreInfo(FilePathView chartFilePath, const MusicGame::PlayResult& playResult, const KscKey& condition)
	{
		FilePath kscFilePath;
		if (!TryConvertChartFilePathToKscPath(chartFilePath, &kscFilePath))
		{
			return false;
		}

		if (playResult.playOption.gaugeType != condition.gaugeType)
		{
			// ゲージの種類は必ず一致するはず
			assert(false && "Gauge type mismatch");
			return false;
		}

		const bool kscFileExists = FileSystem::IsFile(kscFilePath);
		if (kscFileExists)
		{
			// ファイルが存在する場合は既存のファイルを更新する
			return UpdateExistingKscFile(kscFilePath, playResult, condition);
		}
		else
		{
			// ファイルが存在しない場合は今回のプレイ結果を新しいファイルとして書き込む
			return CreateNewKscFile(kscFilePath, playResult, condition);
		}
	}
}
