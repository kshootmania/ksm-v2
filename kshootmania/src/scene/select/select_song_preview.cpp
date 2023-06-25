#include "select_song_preview.hpp"

namespace
{
	constexpr double kFadeInDurationSec = 0.5;
	constexpr double kFadeInDurationSecFirst = 0.25;
	constexpr double kFadeOutDurationSec = 2.0; // HSP版では0.5秒間でのフェードアウトが指定されているが、2秒間毎フレームそれをリクエストしてしまっているため結果的に2秒間でフェードアウトされている
	constexpr double kPreFadeOutStartDurationSec = 2.0;
	constexpr double kDefaultBgmVolumeSpeed = 1.0 / kFadeInDurationSec;
}

SelectSongPreview::SelectSongPreview()
{
	m_defaultBgmStream.lockBegin();
	m_defaultBgmStream.setVolume(0.0);
	m_defaultBgmStream.play();
	m_defaultBgmStream.lockEnd();
}

void SelectSongPreview::update()
{
	if (!m_songPreviewFilename.empty() && (m_songPreviewStartTimer.reachedZero() || m_isFirst))
	{
		// フェードインして再生開始
		m_songPreviewStream = std::make_unique<ksmaudio::Stream>(m_songPreviewFilename.narrow(), m_songPreviewVolume, true, false);
		m_songPreviewStream->lockBegin();
		m_songPreviewStream->seekPosSec(m_songPreviewOffsetSec);
		const double fadeInDurationSec = m_isFirst ? kFadeInDurationSecFirst : kFadeInDurationSec;
		m_songPreviewStream->setFadeIn(fadeInDurationSec);
		m_songPreviewStream->play();
		m_songPreviewStream->lockEnd();

		m_songPreviewStartTimer.reset();

		if (m_isFirst)
		{
			m_defaultBgmVolumeWithFade = 0.0f;
		}
	}

	const bool isPlayingSongPreview = m_songPreviewStream != nullptr;
	if (isPlayingSongPreview)
	{
		const double previewEndSec = m_songPreviewOffsetSec + m_songPreviewDurationSec;
		const double posSec = m_songPreviewStream->posSec();
		if (previewEndSec <= posSec)
		{
			// フェードアウトが終了したら再び最初からフェードインして再生開始
			// (途中の音が鳴らないようロックを挟んでいる)
			m_songPreviewStream->lockBegin();
			m_songPreviewStream->seekPosSec(m_songPreviewOffsetSec);
			m_songPreviewStream->setFadeIn(kFadeInDurationSec);
			m_songPreviewStream->play();
			m_songPreviewStream->lockEnd();
		}
		else if (previewEndSec - kPreFadeOutStartDurationSec <= posSec && posSec < previewEndSec - kPreFadeOutStartDurationSec + kFadeOutDurationSec / 2) // 誤差によって2回フェードアウトしないよう2で割っている
		{
			// フェードアウト
			if (!m_songPreviewStream->isFading())
			{
				m_songPreviewStream->setFadeOut(kFadeOutDurationSec);
			}
		}

		m_selectingStopwatch.reset();

		// TODO: 曲の終端に被ったときにおかしくなるかも知れないので要検証
	}
	else if (m_songPreviewStartTimer.isRunning())
	{
		m_selectingStopwatch.start();
	}

	const bool isWaitingForSongPreview = m_selectingStopwatch.isRunning() && m_selectingStopwatch.elapsed() <= m_songPreviewStartTimer.duration();
	const double targetDefaultBgmVolume = (isPlayingSongPreview || isWaitingForSongPreview) ? 0.0 : 1.0;
	m_defaultBgmVolumeWithFade = MathUtils::LinearDamp(m_defaultBgmVolumeWithFade, targetDefaultBgmVolume, kDefaultBgmVolumeSpeed, Scene::DeltaTime());
	m_defaultBgmStream.setVolume(m_defaultBgmVolumeWithFade);

	m_isFirst = false;
}

void SelectSongPreview::requestSongPreview(FilePathView filename, double offsetSec, double durationSec, double volume)
{
	if (filename == m_songPreviewFilename)
	{
		// ファイル名に変更がない場合は何もしない
		// (前回と同じ音声ファイルの場合はそのまま再生し続ける)
		return;
	}

	if (filename.empty())
	{
		// ファイル名が空の場合はデフォルトBGMを再生する
		requestDefaultBgm();
		return;
	}

	m_defaultBgmStream.setVolume(0.0);
	m_isPlayingDefaultBgm = false;

	m_songPreviewFilename = filename;
	m_songPreviewStream = nullptr;
	m_songPreviewOffsetSec = offsetSec;
	m_songPreviewDurationSec = durationSec;
	m_songPreviewVolume = volume;

	m_songPreviewStartTimer.restart();
}

void SelectSongPreview::requestDefaultBgm()
{
	m_songPreviewFilename.clear();
	m_songPreviewStream = nullptr;
	m_songPreviewStartTimer.reset();
}
