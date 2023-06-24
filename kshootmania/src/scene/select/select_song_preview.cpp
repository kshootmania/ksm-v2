#include "select_song_preview.hpp"

namespace
{
	constexpr double kFadeInDurationSec = 0.5f;
	constexpr double kFadeInDurationSecFirst = 0.25f;
	constexpr double kFadeOutDurationSec = 2.0f; // HSP版では0.5秒間でのフェードアウトが指定されているが、2秒間毎フレームそれをリクエストしてしまっているため結果的に2秒間でフェードアウトされている
	constexpr double kPreFadeOutStartDurationSec = 2.0f;
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
	// TODO: 押下し続けている間はデフォルトBGMを再生する

	if (m_songPreviewStartTimer.reachedZero() || m_isFirst)
	{
		// デフォルトBGMを止める
		m_defaultBgmStream.setVolume(0.0);
		m_isPlayingDefaultBgm = false;

		// フェードインして再生開始
		m_songPreviewStream = std::make_unique<ksmaudio::Stream>(m_songPreviewFilename.narrow(), m_songPreviewVolume, true, false);
		m_songPreviewStream->lockBegin();
		m_songPreviewStream->seekPosSec(m_songPreviewOffsetSec);
		const double fadeInDurationSec = m_isFirst ? kFadeInDurationSecFirst : kFadeInDurationSec;
		m_songPreviewStream->setFadeIn(fadeInDurationSec);
		m_songPreviewStream->play();
		m_songPreviewStream->lockEnd();

		m_songPreviewStartTimer.reset();
		m_isFirst = false;
	}

	if (m_songPreviewStream != nullptr)
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

		// TODO: ループの再開タイミングをHSP版と同じにする
		// TODO: 曲の終端に被ったときにおかしくなるかも知れないので要検証
	}
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
	if (m_isPlayingDefaultBgm)
	{
		// 既にデフォルトBGMが再生されている場合は何もしない
		return;
	}

	m_songPreviewFilename.clear();
	m_songPreviewStream = nullptr;

	// フェードインして再生開始
	m_defaultBgmStream.setFadeIn(kFadeInDurationSec, 1.0);
	m_isPlayingDefaultBgm = true;
}
