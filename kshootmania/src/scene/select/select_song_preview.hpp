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

	Timer m_songPreviewStartTimer{ 0.2s, StartImmediately::No };

	Timer m_defaultBgmStartTimer{ 0.25s, StartImmediately::No };

public:
	SelectSongPreview();

	void update();

	void requestSongPreview(FilePathView filename, double offsetSec, double durationSec, double volume);

	void requestDefaultBgm();
};
