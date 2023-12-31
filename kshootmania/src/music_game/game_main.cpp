#include "game_main.hpp"
#include "game_defines.hpp"
#include "kson/kson.hpp"

namespace MusicGame
{
	void GameMain::updateStatus()
	{
		// 曲の音声の更新
		m_bgm.update();

		// 再生時間と現在のBPMを取得
		const double currentTimeSec = m_bgm.posSec();
		const kson::Pulse currentPulse = kson::SecToPulse(currentTimeSec, m_chartData.beat, m_timingCache);
		const double currentPulseDouble = kson::SecToPulseDouble(currentTimeSec, m_chartData.beat, m_timingCache);
		const double currentBPM = kson::TempoAt(currentPulse, m_chartData.beat);
		m_gameStatus.currentTimeSec = currentTimeSec;
		m_gameStatus.currentPulse = currentPulse;
		m_gameStatus.currentPulseDouble = currentPulseDouble;
		m_gameStatus.currentBPM = currentBPM;

		// 視点変更を更新
		// (CamStatusにノーツイベントによる値が相対的に反映されるので、判定の更新より先に実行する必要がある)
		m_camSystem.update(m_chartData, m_gameStatus.currentPulse);
		m_viewStatus.camStatus = m_camSystem.status();

		// 傾きを更新
		m_highwayTilt.update(m_chartData, m_gameStatus.currentPulse);
		m_viewStatus.tiltRadians = m_highwayTilt.radians();

		// 判定の更新
		m_judgmentMain.update(m_chartData, m_gameStatus, m_viewStatus);
	}

	void GameMain::updateHighwayScroll()
	{
		// ハイスピードを更新
		if (m_isFirstUpdate)
		{
			// HighwayScrollはHispeedSettingMenuの更新に必要だが、事前に一度は更新しておかないとBPMが入らないので、初回は追加で先に更新
			// TODO: 消したい
			m_highwayScroll.update(m_hispeedSettingMenu.hispeedSetting(), m_gameStatus.currentBPM);
		}
		m_hispeedSettingMenu.update(m_highwayScroll);
		m_highwayScroll.update(m_hispeedSettingMenu.hispeedSetting(), m_gameStatus.currentBPM);
	}

	GameMain::GameMain(const GameCreateInfo& createInfo)
		: m_chartFilePath(createInfo.chartFilePath)
		, m_parentPath(FileSystem::ParentPath(createInfo.chartFilePath))
		, m_chartData(kson::LoadKSHChartData(createInfo.chartFilePath.narrow()))
		, m_timingCache(kson::CreateTimingCache(m_chartData.beat))
		, m_judgmentMain(m_chartData, m_timingCache, createInfo.gaugeType)
		, m_highwayScroll(m_chartData)
		, m_bgm(FileSystem::PathAppend(m_parentPath, Unicode::FromUTF8(m_chartData.audio.bgm.filename)), m_chartData.audio.bgm.vol, static_cast<double>(m_chartData.audio.bgm.offset) / 1000)
		, m_assistTick(createInfo.assistTickEnabled)
		, m_laserSlamSE(m_chartData)
		, m_audioEffectMain(m_bgm, m_chartData, m_timingCache)
		, m_graphicsMain(m_chartData, m_parentPath, createInfo.gaugeType)
	{
	}

	void GameMain::start()
	{
		m_bgm.seekPosSec(-TimeSecBeforeStart(false/* TODO: movie */));
		m_bgm.play();
	}

	void GameMain::update()
	{
		// 状態更新
		updateStatus();

		// スクロールの更新
		updateHighwayScroll();

		// 音声エフェクトの更新
		std::array<Optional<bool>, kson::kNumFXLanesSZ> longFXPressed;
		for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
		{
			longFXPressed[i] = m_gameStatus.fxLaneStatus[i].longNotePressed;
		}
		std::array<bool, kson::kNumLaserLanesSZ> laserIsOnOrNone;
		for (std::size_t i = 0U; i < kson::kNumLaserLanesSZ; ++i)
		{
			const auto& laneStatus = m_gameStatus.laserLaneStatus[i];
			laserIsOnOrNone[i] = !laneStatus.noteCursorX.has_value() || laneStatus.isCursorInCriticalJudgmentRange();
		}
		m_audioEffectMain.update(m_bgm, m_chartData, m_timingCache, {
			.longFXPressed = longFXPressed,
			.laserIsOnOrNone = laserIsOnOrNone,
		});

		// 効果音の更新
		const double currentTimeSec = m_bgm.posSec();
		m_assistTick.update(m_chartData, m_timingCache, currentTimeSec);
		m_laserSlamSE.update(m_chartData, m_gameStatus);

		// グラフィックの更新
		m_graphicsMain.update(m_viewStatus);

		m_isFirstUpdate = false;
	}

	void GameMain::draw() const
	{
		// HighwayScrollのコンテキスト
		// (HighwayScrollからの座標取得の引数を省略するためのもの)
		const Scroll::HighwayScrollContext highwayScrollContext(&m_highwayScroll, &m_chartData.beat, &m_timingCache, &m_gameStatus);

		// 描画実行
		m_graphicsMain.draw(m_chartData, m_gameStatus, m_viewStatus, highwayScrollContext);
	}

	void GameMain::lockForExit()
	{
		m_judgmentMain.lockForExit();
	}

	void GameMain::terminate()
	{
		m_hispeedSettingMenu.saveToConfigIni();
	}

	FilePathView GameMain::chartFilePath() const
	{
		return m_chartFilePath;
	}

	const kson::ChartData& GameMain::chartData() const
	{
		return m_chartData;
	}

	PlayResult GameMain::playResult() const
	{
		return m_judgmentMain.playResult();
	}
}
