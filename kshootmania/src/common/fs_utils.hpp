#pragma once

/// @brief ファイルシステム関連の関数群
namespace FsUtils
{
	/// @brief 特殊フォルダ内の相対パスからフルパスを取得
	/// @param folder 特殊フォルダの種類
	/// @param relativePath 特殊フォルダ内の相対パス
	/// @return フルパス
	FilePath GetFullPathInFolder(SpecialFolder folder, FilePathView relativePath);

	/// @brief 実行ファイルがあるディレクトリのフルパスを取得
	/// @return フルパス
	FilePath AppDirectoryPath();

	/// @brief ディレクトリパスからディレクトリ名を取得
	/// @param directoryPath ディレクトリパス(ファイルパスは不可)
	/// @return ディレクトリ名
	String DirectoryNameByDirectoryPath(FilePathView directoryPath);

	/// @brief ファイルパスから拡張子を除去
	/// @param path ファイルパス
	/// @return 拡張子を除去したファイルパス
	String EliminateExtension(FilePathView path);
}
