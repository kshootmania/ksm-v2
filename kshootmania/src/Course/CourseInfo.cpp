#include "CourseInfo.hpp"
#include "Common/FsUtils.hpp"
#include "Common/Encoding.hpp"

namespace
{
	bool StartsWith(StringView str, StringView prefix)
	{
		return str.starts_with(prefix);
	}

	bool IsInvalidRelativePath(StringView path)
	{
		// 絶対パスや..を含むパスは受け付けない
		return path.starts_with(U'/') || path.starts_with(U'\\') || path.includes(U':') || path.includes(U"..");
	}

	FilePath ResolveChartPath(StringView relativePath, FilePathView songsDir)
	{
		// 相対パスを絶対パスに変換
		const FilePath fullPath = FileSystem::PathAppend(songsDir, relativePath);

		// ファイルが存在するかチェック
		if (FileSystem::IsFile(fullPath))
		{
			return fullPath;
		}

		// 指定された拡張子のファイルが見つからない場合、他方の拡張子も試す
		if (FsUtils::HasChartExtension(fullPath))
		{
			const String basePath = FsUtils::EliminateExtension(fullPath);
			const FilePath altPath = FsUtils::HasKsonExtension(fullPath)
				? basePath + U"." + kKSHExtension
				: basePath + U"." + kKSONExtension;
			if (FileSystem::IsFile(altPath))
			{
				return altPath;
			}
		}

		return U"";
	}
}

Optional<CourseInfo> CourseInfo::Load(FilePathView kcoFilePath)
{
	if (!FileSystem::IsFile(kcoFilePath))
	{
		return none;
	}

	CourseInfo courseInfo;
	courseInfo.filePath = FilePath{ kcoFilePath };

	const Array<String> lines = Encoding::ReadTextFileLinesShiftJISOrUTF8BasedOnBOM(kcoFilePath);
	if (lines.isEmpty())
	{
		return none;
	}

	bool inChartList = false;
	const FilePath songsDir = FsUtils::SongsDirectoryPath();

	for (const String& untrimmedLine : lines)
	{
		const String line = untrimmedLine.trimmed();

		if (line.isEmpty())
		{
			continue;
		}

		if (line == U"--")
		{
			inChartList = true;
			continue;
		}

		if (!inChartList)
		{
			if (StartsWith(line, U"title="))
			{
				courseInfo.title = line.substr(6);
			}
			else if (StartsWith(line, U"title_img="))
			{
				courseInfo.titleImgPath = line.substr(10);
			}
			else if (StartsWith(line, U"information="))
			{
				courseInfo.information = line.substr(12);
			}
			else if (StartsWith(line, U"icon="))
			{
				courseInfo.iconPath = line.substr(5);

				// 拡張子なしの場合はimgs/icon内の画像を使用
				if (FileSystem::Extension(courseInfo.iconPath).isEmpty())
				{
					courseInfo.iconPath = FileSystem::PathAppend(U"imgs/icon", courseInfo.iconPath + U".png");
				}
			}
		}
		else
		{
			// 譜面リスト読み込み
			if (line.starts_with(U"[") && line.ends_with(U"]"))
			{
				String chartPath = line.substr(1, line.length() - 2);

				// 不正なパス(絶対パスや..を含むパス)はスキップ
				if (IsInvalidRelativePath(chartPath))
				{
					Logger << U"[ksm warning] Invalid chart path in course file: {}"_fmt(chartPath);
					continue;
				}

				CourseChartEntry entry;
				entry.relativePath = chartPath;

				entry.absolutePath = ResolveChartPath(chartPath, songsDir);
				entry.exists = !entry.absolutePath.isEmpty();
				if (!entry.exists)
				{
					Logger << U"[ksm warning] Chart file not found in course: {}"_fmt(chartPath);
				}
				courseInfo.charts.push_back(entry);
			}
		}
	}

	if (courseInfo.charts.isEmpty())
	{
		return none;
	}

	return courseInfo;
}
