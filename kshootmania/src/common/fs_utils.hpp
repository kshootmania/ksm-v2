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
}
