#include "FolderConfIni.hpp"
#include "KSMIniData.hpp"
#include "Common/FsUtils.hpp"

namespace
{
	constexpr StringView kVolumeKey = U"volume";
	constexpr StringView kOffsetKey = U"offset";
	constexpr StringView kTweetOptionKey = U"tweet_option";
	constexpr int32 kDefaultVolume = 100;
	constexpr int32 kDefaultOffset = 0;

	HashTable<FilePath, FolderConfIni> s_cache;

	// 譜面ファイルパスからパッケージフォルダのパスを取得
	FilePath GetPackageFolderPath(FilePathView chartFilePath)
	{
		const String relativePath = FsUtils::RelativePathFromSongsDir(chartFilePath);
		if (relativePath.empty())
		{
			return {};
		}

		// 相対パスの最初のセグメント(パッケージフォルダ名)を抽出
		const std::size_t separatorPos = relativePath.indexOfAny(U"/\\");
		const String packageFolderName = separatorPos == String::npos
			? relativePath
			: relativePath.substr(0, separatorPos);

		return FileSystem::PathAppend(FsUtils::SongsDirectoryPath(), packageFolderName);
	}
}

FolderConfIni FolderConfIni::Load(FilePathView chartFilePath)
{
	const FilePath packageFolderPath = GetPackageFolderPath(chartFilePath);
	if (packageFolderPath.empty())
	{
		return {};
	}

	// キャッシュを確認
	if (s_cache.contains(packageFolderPath))
	{
		return s_cache.at(packageFolderPath);
	}

	FolderConfIni config;

	const FilePath confIniPath = FileSystem::PathAppend(packageFolderPath, U"conf.ini");
	if (!FileSystem::IsFile(confIniPath))
	{
		// ファイルが存在しない場合はデフォルト値をキャッシュして返す
		s_cache.emplace(packageFolderPath, config);
		return config;
	}

	// conf.iniを読み込み
	const KSMIniData iniData(confIniPath);

	// volume
	const int32 volume = iniData.getInt(kVolumeKey, kDefaultVolume);
	if (volume < 0)
	{
		Logger << U"[ksm warning] Invalid volume value in '{}': {} (must be >= 0)"_fmt(confIniPath, volume);
		config.volumeScale = kDefaultVolume / 100.0;
	}
	else
	{
		config.volumeScale = volume / 100.0;
	}

	// offset
	config.offsetMs = iniData.getInt(kOffsetKey, kDefaultOffset);

	// tweet_option
	config.tweetOption = iniData.getString(kTweetOptionKey);

	s_cache.emplace(packageFolderPath, config);
	return config;
}

void FolderConfIni::ClearCache()
{
	s_cache.clear();
}
