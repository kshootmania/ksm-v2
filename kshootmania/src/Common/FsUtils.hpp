#pragma once

/// @brief ファイルシステム関連の関数群
namespace FsUtils
{
#if defined(__linux__)
	/// @brief FsUtils用のModulePathを初期化(Linux用)
	/// @remark プログラム起動直後のカレントディレクトリが未変更のタイミングで一度だけ呼び出す
	void InitModulePathForLinux();
#endif

	/// @brief 特殊フォルダ内の相対パスからフルパスを取得
	/// @param folder 特殊フォルダの種類
	/// @param relativePath 特殊フォルダ内の相対パス
	/// @return フルパス
	[[nodiscard]]
	FilePath GetFullPathInFolder(SpecialFolder folder, FilePathView relativePath);

	/// @brief 書き込み可能なデータディレクトリのフルパスを取得
	/// @details macOSの場合は~/Library/Application Support/kshootmania/、それ以外は実行ファイルがあるディレクトリ
	/// @return フルパス
	[[nodiscard]]
	FilePath AppDataDirectoryPath();

	/// @brief リソースディレクトリのフルパスを取得
	/// @details macOSの場合は.app/Contents/Resources/、それ以外はAppDataDirectoryPath()と同じ
	/// @return フルパス
	[[nodiscard]]
	FilePath ResourceDirectoryPath();

	/// @brief scoreフォルダのフルパスを取得
	/// @return フルパス
	[[nodiscard]]
	FilePath ScoreDirectoryPath();

	/// @brief songsフォルダのフルパスを取得
	/// @return フルパス
	[[nodiscard]]
	FilePath SongsDirectoryPath();

	/// @brief デフォルトsongsフォルダ(songs_default)のフルパスを取得
	/// @details リソースディレクトリ配下のsongs_default
	/// @return フルパス
	[[nodiscard]]
	FilePath SongsDefaultDirectoryPath();

	/// @brief coursesフォルダのフルパスを取得
	/// @return フルパス
	[[nodiscard]]
	FilePath CoursesDirectoryPath();

	/// @brief courses/scoreフォルダのフルパスを取得
	/// @return フルパス
	[[nodiscard]]
	FilePath CourseScoreDirectoryPath();

	/// @brief 指定されたリソースフォルダのフルパスを取得
	/// @param folderName リソースフォルダ名 (例: U"imgs", U"lang", U"se", U"shaders", U"ui")
	/// @return フルパス
	[[nodiscard]]
	FilePath GetResourcePath(FilePathView folderName);

	/// @brief ディレクトリパスからディレクトリ名を取得
	/// @param directoryPath ディレクトリパス(ファイルパスは不可)
	/// @return ディレクトリ名
	[[nodiscard]]
	String DirectoryNameByDirectoryPath(FilePathView directoryPath);

	/// @brief ファイルパスから拡張子を除去
	/// @param path ファイルパス
	/// @return 拡張子を除去したファイルパス
	[[nodiscard]]
	String EliminateExtension(FilePathView path);

	/// @brief songsフォルダからの相対パスを取得
	/// @param fullPath フルパス
	/// @return songsフォルダからの相対パス
	[[nodiscard]]
	String RelativePathFromSongsDir(FilePathView fullPath);
}
