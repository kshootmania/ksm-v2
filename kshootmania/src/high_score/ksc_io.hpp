#pragma once
#include "high_score_info.hpp"
#include "ksc_key.hpp"

namespace KscIo
{
	/// @brief ハイスコア情報を読み込む
	/// @param chartFilePath 譜面ファイルのパス(kscファイルのパスではないので注意)
	/// @param condition 読み込むハイスコア情報の条件
	/// @return 読み込んだハイスコア情報
	HighScoreInfo ReadHighScoreInfo(FilePathView chartFilePath, const KscKey& condition);

	/// @brief ハイスコア情報を書き込む
	/// @param chartFilePath 譜面ファイルのパス(kscファイルのパスではないので注意)
	/// @param playResult プレイ結果
	/// @param condition 書き込むハイスコア情報の条件
	/// @return 書き込みに成功した場合はtrue, そうでなければfalse
	bool WriteHighScoreInfo(FilePathView chartFilePath, const MusicGame::PlayResult& playResult, const KscKey& condition);
}
