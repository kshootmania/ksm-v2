#pragma once

/// @brief フォルダ単位のconf.ini設定
/// @details songsディレクトリ直下のパッケージフォルダにconf.iniを配置することで、
///          そのパッケージ内の全楽曲に一括で設定を適用できる
struct FolderConfIni
{
	double volumeScale = 1.0;
	int32 offsetMs = 0;
	String tweetOption;

	/// @brief 譜面ファイルパスからconf.iniを読み込む
	/// @param chartFilePath 譜面ファイルのフルパス
	/// @return FolderConfIni(ファイルが存在しない場合はデフォルト値)
	[[nodiscard]]
	static FolderConfIni Load(FilePathView chartFilePath);

	/// @brief キャッシュをクリア
	static void ClearCache();
};
