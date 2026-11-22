#pragma once
#include <Siv3D.hpp>

namespace MusicGame::Graphics::MovieUtils
{
	/// @brief 動画ファイルの読み込み
	/// @param moviePath 動画ファイルのパス
	/// @param fnLoad パスを受け取って読み込みを行い、成功したかを返す関数
	/// @return 読み込みに成功したファイルのパス(失敗した場合はnone)
	/// @remark 失敗した場合や存在しない場合、拡張子が.mp4でなければ.mp4に変えて再試行する
	[[nodiscard]]
	Optional<FilePath> LoadWithFallback(const FilePath& moviePath, const std::function<bool(const FilePath&)>& fnLoad);

	/// @brief 実際のフレームレートが報告値の何倍かを取得
	/// @remark OpenSiv3DのVideoReaderがwmvを本来と異なる30fpsとして報告する場合があるため、その場合の補正用
	[[nodiscard]]
	double FpsScale(const FilePath& loadedPath, double reportedFps);
}
