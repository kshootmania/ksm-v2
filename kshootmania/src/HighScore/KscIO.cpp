#include "KscIO.hpp"
#include "Common/FsUtils.hpp"
#include "Ini/ConfigIni.hpp"
#include "Course/CoursePlayState.hpp"
#include "Course/CoursePlayResult.hpp"

namespace KscIO
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

			// Note: ハイスコア情報(.ksc)は、.kshと.ksonで共通とする
			// (例:"song.ksh"と"song.kson"は両方"song.ksc"を参照)
			if (!FsUtils::HasChartExtension(chartFilePath))
			{
				return false;
			}

			// サブフォルダの存在を考慮する必要があるため、songsフォルダからの相対パスを使用してkscファイルのパスを生成
			const auto relativeChartFilePath = FsUtils::RelativePathFromSongsDir(chartFilePath); // TODO: songsフォルダ以外が指定可能になったら要修正
			const String currentPlayer{ ConfigIni::GetString(ConfigIni::Key::kCurrentPlayer) };
			*pFilePath = FileSystem::PathAppend(FsUtils::ScoreDirectoryPath(), U"{}/{}.ksc"_fmt(currentPlayer, FsUtils::EliminateExtension(relativeChartFilePath)));
			return true;
		}

		bool TryConvertCourseFilePathToKscPath(FilePathView courseFilePath, FilePath* pFilePath)
		{
			if (pFilePath == nullptr)
			{
				assert(false && "pFilePath must not be NULL");
				return false;
			}

			const auto extension = FileSystem::Extension(courseFilePath);
			if (extension != U"kco")
			{
				return false;
			}

			const String currentPlayer{ ConfigIni::GetString(ConfigIni::Key::kCurrentPlayer) };
			const FilePath coursesDir = FsUtils::CoursesDirectoryPath();
			const FilePath songsDir = FsUtils::SongsDirectoryPath();
			const FilePath courseScoreDir = FsUtils::CourseScoreDirectoryPath();

			// coursesフォルダからの相対パスを試す
			auto relativePath = FileSystem::RelativePath(courseFilePath, coursesDir);
			if (relativePath.isEmpty() || relativePath.starts_with(U".."))
			{
				// coursesフォルダ内にない場合はsongsフォルダからの相対パスを試す
				relativePath = FsUtils::RelativePathFromSongsDir(courseFilePath);
				if (relativePath.isEmpty() || relativePath.starts_with(U".."))
				{
					// どちらのフォルダにも属していない
					return false;
				}
			}

			*pFilePath = FileSystem::PathAppend(courseScoreDir, U"{}/{}.ksc"_fmt(currentPlayer, FsUtils::EliminateExtension(relativePath)));
			return true;
		}

		// kscファイルの行からgaugeType文字列部分を除去したキーを取得
		// (選曲画面表示時はゲージタイプは合算して表示するため)
		Optional<String> GetKeyWithoutGaugeTypeFromLine(StringView line)
		{
			const size_t eqPos = line.indexOf(U'=');
			if (eqPos == String::npos)
			{
				return none;
			}
			const StringView keyPart = line.substr(0, eqPos);
			const size_t commaPos = keyPart.indexOf(U',');
			if (commaPos == String::npos)
			{
				return none;
			}
			return String{ keyPart.substr(commaPos + 1) };
		}

		// kscファイルの行からgaugeType文字列を取得
		Optional<StringView> GetGaugeTypeStrFromLine(StringView line)
		{
			const size_t commaPos = line.indexOf(U',');
			if (commaPos == String::npos)
			{
				return none;
			}
			return line.substr(0, commaPos);
		}

		// kscファイルの行からvalue文字列を取得
		Optional<StringView> GetValueStrFromLine(StringView line)
		{
			const size_t eqPos = line.indexOf(U'=');
			if (eqPos == String::npos)
			{
				return none;
			}
			return line.substr(eqPos + 1);
		}

		void ReadAllHighScoreInfoFromKscFile(FilePathView kscFilePath, HashTable<String, HighScoreInfo>* pHighScoreInfoMap)
		{
			pHighScoreInfoMap->clear();

			if (!FileSystem::Exists(kscFilePath))
			{
				return;
			}

			TextReader reader(kscFilePath);
			if (!reader)
			{
				return;
			}

			String line;
			while (reader.readLine(line))
			{
				const auto keyWithoutGauge = GetKeyWithoutGaugeTypeFromLine(line);
				if (!keyWithoutGauge)
				{
					continue;
				}

				const auto gaugeTypeStr = GetGaugeTypeStrFromLine(line);
				const auto valueStr = GetValueStrFromLine(line);
				if (!gaugeTypeStr || !valueStr)
				{
					continue;
				}

				HighScoreInfo& info = (*pHighScoreInfoMap)[*keyWithoutGauge];
				const KscValue kscValue = KscValue::FromString(String{ *valueStr });

				if (*gaugeTypeStr == U"easy")
				{
					info.easyGauge = kscValue;
				}
				else if (*gaugeTypeStr == U"normal")
				{
					info.normalGauge = kscValue;
				}
				else if (*gaugeTypeStr == U"hard")
				{
					info.hardGauge = kscValue;
				}
			}
		}

		bool TryReadHighScoreInfo(FilePathView kscFilePath, const KscKey& condition, HighScoreInfo* pHighScoreInfo)
		{
			if (pHighScoreInfo == nullptr)
			{
				assert(false && "pHighScoreInfo must not be NULL");
				return false;
			}

			if (!FileSystem::Exists(kscFilePath))
			{
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
				const KscValue newKscValue = playResult.playOption.gameMode == MusicGame::GameMode::kCourseMode
					? origKscValue.applyPlayResultForCourse(playResult)
					: origKscValue.applyPlayResult(playResult);
				const String replace = search + newKscValue.toString();

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

	void ReadAllHighScoreInfo(FilePathView chartFilePath, HashTable<String, HighScoreInfo>* pHighScoreInfoMap)
	{
		FilePath kscFilePath;
		if (!TryConvertChartFilePathToKscPath(chartFilePath, &kscFilePath))
		{
			pHighScoreInfoMap->clear();
			return;
		}

		ReadAllHighScoreInfoFromKscFile(kscFilePath, pHighScoreInfoMap);
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

	HighScoreInfo ReadCourseHighScoreInfo(FilePathView courseFilePath, const KscKey& condition)
	{
		FilePath kscFilePath;
		if (!TryConvertCourseFilePathToKscPath(courseFilePath, &kscFilePath))
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

	void ReadAllCourseHighScoreInfo(FilePathView courseFilePath, HashTable<String, HighScoreInfo>* pHighScoreInfoMap)
	{
		FilePath kscFilePath;
		if (!TryConvertCourseFilePathToKscPath(courseFilePath, &kscFilePath))
		{
			pHighScoreInfoMap->clear();
			return;
		}

		ReadAllHighScoreInfoFromKscFile(kscFilePath, pHighScoreInfoMap);
	}

	bool WriteCourseHighScoreInfo(FilePathView courseFilePath, const CoursePlayState& courseState)
	{
		FilePath kscFilePath;
		if (!TryConvertCourseFilePathToKscPath(courseFilePath, &kscFilePath))
		{
			return false;
		}

		const CoursePlayResult playResult = courseState.coursePlayResult();
		const KscKey& condition = courseState.kscKey();

		// 既存のkscファイルを読み込んで、今回のプレイ結果を反映したkscファイルの文字列を生成
		String newKscFileContent;
		{
			const String search = condition.toString() + U"=";

			// 既存のハイスコア情報を読み込み
			KscValue origKscValue;
			if (FileSystem::Exists(kscFilePath))
			{
				TextReader reader(kscFilePath);
				if (reader)
				{
					String line;
					while (reader.readLine(line))
					{
						if (line.starts_with(search))
						{
							const String valueStr = line.substr(search.length());
							origKscValue = KscValue::FromString(valueStr);
							break;
						}
					}
				}
			}

			// 新しいハイスコア情報を生成
			const KscValue newKscValue
			{
				.score = Max(origKscValue.score, playResult.avgScore),
				.achievement = Max(origKscValue.achievement, playResult.achievement()),
				.grade = Max(origKscValue.grade, Grade::kNoGrade),
				.percent = Max(origKscValue.percent, playResult.gaugePercentForHighScore()),
				.maxCombo = Max(origKscValue.maxCombo, playResult.maxCombo),
				.playCount = origKscValue.playCount + 1,
				.clearCount = origKscValue.clearCount + (playResult.achievement() >= Achievement::kCleared ? 1 : 0),
				.fullComboCount = origKscValue.fullComboCount + (playResult.achievement() >= Achievement::kFullCombo ? 1 : 0),
				.perfectCount = origKscValue.perfectCount + (playResult.achievement() >= Achievement::kPerfect ? 1 : 0),
			};
			const String replace = search + newKscValue.toString();

			// ファイルが存在する場合は既存の内容を更新
			bool found = false;
			if (FileSystem::Exists(kscFilePath))
			{
				TextReader reader(kscFilePath);
				if (!reader)
				{
					return false;
				}

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
			}

			if (!found)
			{
				// 対象の行が見つからなかった場合は新しい行にハイスコア情報を追記
				newKscFileContent += replace + U"\n";
			}
		}

		// ディレクトリが存在しない場合は作成
		const FilePath kscDir = FileSystem::ParentPath(kscFilePath);
		if (!FileSystem::Exists(kscDir))
		{
			FileSystem::CreateDirectories(kscDir);
		}

		// ファイルを書き込み
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
