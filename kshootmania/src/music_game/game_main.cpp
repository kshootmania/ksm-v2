#include "game_main.hpp"
#include "game_defines.hpp"
#include "kson/kson.hpp"

namespace MusicGame
{
	void GameMain::updateGameStatus()
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

		// 判定の更新
		m_judgmentMain.update(m_chartData, m_gameStatus, m_viewStatus);
	}

	void GameMain::updateViewStatus()
	{
		// 傾きを更新
		const double leftLaserValue = kson::GraphSectionValueAtWithDefault(m_chartData.note.laser[0], m_gameStatus.currentPulse, 0.0); // range: [0, +1]
		const double rightLaserValue = kson::GraphSectionValueAtWithDefault(m_chartData.note.laser[1], m_gameStatus.currentPulse, 1.0) - 1.0; // range: [-1, 0]
		const double tiltFactor = leftLaserValue + rightLaserValue; // range: [-1, +1]
		m_highwayTilt.update(tiltFactor, m_chartData, m_gameStatus.currentPulse);
		m_viewStatus.tiltRadians = m_highwayTilt.radians();

		// 視点変更を更新
		m_camSystem.update(m_chartData, m_gameStatus.currentPulse);
		m_viewStatus.camStatus = m_camSystem.status();
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
		: m_parentPath(FileSystem::ParentPath(createInfo.chartFilePath))
		, m_chartData(kson::LoadKSHChartData(createInfo.chartFilePath.narrow()))
		, m_timingCache(kson::CreateTimingCache(m_chartData.beat))
		, m_judgmentMain(m_chartData, m_timingCache)
		, m_highwayScroll(m_chartData)
		, m_bgm(FileSystem::PathAppend(m_parentPath, Unicode::FromUTF8(m_chartData.audio.bgm.filename)), m_chartData.audio.bgm.vol, static_cast<double>(m_chartData.audio.bgm.offset) / 1000)
		, m_assistTick(createInfo.assistTickEnabled)
		, m_laserSlamSE(m_chartData)
		, m_audioEffectMain(m_bgm, m_chartData, m_timingCache)
		, m_graphicsMain(m_chartData, m_parentPath)
	{
	}

	void GameMain::start()
	{
		m_bgm.seekPosSec(-TimeSecBeforeStart(false/* TODO: movie */));
		m_bgm.play();

		kson::SaveKSONChartData("hogehoge.kson", m_chartData);
	}

	void GameMain::update()
	{
		// ゲームステータスの更新とノーツ判定
		updateGameStatus();

		// ビューステータスの更新
		updateViewStatus();

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

	void GameMain::terminate()
	{
		m_hispeedSettingMenu.saveToConfigIni();
	}
}
