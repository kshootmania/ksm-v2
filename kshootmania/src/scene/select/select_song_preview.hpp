#pragma once
#include "kson/chart_data.hpp"
#include "ksmaudio/ksmaudio.hpp"

class SelectSongPreview
{
private:
	ksmaudio::Stream m_defaultBgmStream{"se/sel_bgm.ogg", 1.0, false, false, true};

	bool m_isPlayingDefaultBgm = false;

	String m_songPreviewFilename;

	std::unique_ptr<ksmaudio::Stream> m_songPreviewStream;

	double m_songPreviewOffsetSec = 0.0;

	double m_songPreviewDurationSec = 0.0;

	double m_songPreviewVolume = 1.0;

	double m_defaultBgmVolumeWithFade = 1.0;

	/// @brief 楽曲プレビュー開始までに猶予を持たせるためのタイマー
	/// @note 猶予を持たせている理由: カーソル移動直後に再生開始すると負荷でカーソル移動がもたつくため。また、連続したカーソル移動中はデフォルトBGMを再生したいため。
	Timer m_songPreviewStartTimer{ 0.2s, StartImmediately::No };

	/// @brief カーソル移動を始めてからの時間を計測するストップウォッチ
	/// @note カーソル移動中以外は停止状態になっている
	Stopwatch m_selectingStopwatch{ StartImmediately::No };

	bool m_isFirst = true;

public:
	SelectSongPreview();

	void update();

	void requestSongPreview(FilePathView filename, double offsetSec, double durationSec, double volume);

	void requestDefaultBgm();
};
