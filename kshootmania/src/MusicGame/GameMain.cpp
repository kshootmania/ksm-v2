#include "GameMain.hpp"
#include "GameDefines.hpp"
#include "TurnUtil.hpp"
#include "PlayModeUtil.hpp"
#include "kson/kson.hpp"
#include "Input/PlatformKey.hpp"

namespace MusicGame
{
	namespace
	{
		constexpr double kPlayFinishFadeOutStartSec = 2.4; // TODO: HARD落ちした場合は赤色表示を加えた上で4.8秒にする
		constexpr double kTestPlaySeekMarginSec = 1.0; // テストプレイで開始小節より前にシークするマージン(秒)

		Audio::BGM CreateBGM(const kson::ChartData& chartData, const FilePath& parentPath, const PlayOption& playOption, const FolderConfIni& folderConfIni)
		{
			const FilePath filePath = FileSystem::PathAppend(parentPath, Unicode::FromUTF8(chartData.audio.bgm.filename));
			const double volume = chartData.audio.bgm.vol * folderConfIni.volumeScale;
			const double chartOffsetMs = static_cast<double>(chartData.audio.bgm.offset + folderConfIni.offsetMs);
			const double playbackSpeed = playOption.nonZeroPlaybackSpeed();
			const SecondsF offset{ (chartOffsetMs / playbackSpeed + playOption.effectiveGlobalOffsetMs()) / 1000.0 };
			const Audio::LegacyAudioFPMode legacyMode = Audio::DetermineLegacyAudioFPMode(chartData, parentPath);

			return Audio::BGM{ filePath, volume, offset, legacyMode, chartData, parentPath, playOption.playbackSpeed };
		}

		bool ShouldStartFadeOut(const GameStatus& gameStatus)
		{
			if (!gameStatus.playFinishStatus.has_value())
			{
				return false;
			}

			const double secSincePlayFinishPrev = gameStatus.currentTimeSec - gameStatus.playFinishStatus->finishTimeSec;
			return secSincePlayFinishPrev >= kPlayFinishFadeOutStartSec;
		}

		double GetInitialBPM(const kson::ChartData& chartData)
		{
			return chartData.beat.bpm.contains(0) ? chartData.beat.bpm.at(0) : kDefaultBPM;
		}

		// 譜面データを読み込み、Turn変換とPlayModeフィルタを適用
		kson::ChartData LoadChartDataWithTurn(const GameCreateInfo& createInfo, std::array<HashSet<kson::Pulse>, kson::kNumLaserLanesSZ>* pLaserCurvedPulses)
		{
			auto chartData = FsUtils::HasKsonExtension(createInfo.chartFilePath)
				? kson::LoadKsonChartData(createInfo.chartFilePath.toUTF8())
				: kson::LoadKshChartData(createInfo.chartFilePath.toUTF8());

			// Turn変換を適用
			const TurnTable turnTable = MakeTurnTable(createInfo.playOption.turnMode);
			ApplyTurnTable(chartData, turnTable);

			// Off/Hideモードフィルタを適用
			ApplyPlayModeFilter(chartData, createInfo.playOption);

			// テストプレイの場合、開始小節より手前のノーツを削除
			if (createInfo.playOption.isTestPlayWithStartMeasure())
			{
				const auto tempTimingCache = kson::CreateTimingCache(chartData.beat);
				const int32 startMeasure = *createInfo.playOption.testPlayOption->startMeasure;
				const kson::Pulse startPulse = kson::MeasureIdxToPulse(startMeasure, chartData.beat, tempTimingCache);

				// BT/FXノーツ削除
				for (auto& lane : chartData.note.bt)
				{
					lane.erase(lane.begin(), lane.lower_bound(startPulse));
				}
				for (auto& lane : chartData.note.fx)
				{
					lane.erase(lane.begin(), lane.lower_bound(startPulse));
				}

				// LASERセクションのトリミング
				for (auto& lane : chartData.note.laser)
				{
					auto it = lane.begin();
					while (it != lane.end() && it->first < startPulse)
					{
						const kson::Pulse sectionY = it->first;
						const kson::LaserSection& section = it->second;
						const kson::RelPulse relStart = startPulse - sectionY;

						// セクション全体が開始位置以前にある場合はセクションごと削除
						if (section.v.rbegin()->first <= relStart)
						{
							it = lane.erase(it);
							continue;
						}

						// 開始位置以降の点でセクションを作り直す
						const double valueAtStart = kson::GraphValueAt(section.v, relStart);
						kson::LaserSection newSection;
						newSection.w = section.w;
						newSection.v[0] = kson::GraphPoint(valueAtStart);
						for (auto pointIt = section.v.upper_bound(relStart); pointIt != section.v.end(); ++pointIt)
						{
							newSection.v[pointIt->first - relStart] = pointIt->second;
						}

						it = lane.erase(it);
						lane[startPulse] = std::move(newSection);
						it = lane.upper_bound(startPulse);
					}
				}
			}

			// 曲線を持つレーザー点のPulseを列挙
			if (pLaserCurvedPulses != nullptr)
			{
				for (std::size_t laneIdx = 0; laneIdx < kson::kNumLaserLanesSZ; ++laneIdx)
				{
					for (const auto& [y, section] : chartData.note.laser[laneIdx])
					{
						for (const auto& [ry, point] : section.v)
						{
							if (!point.curve.isLinear())
							{
								pLaserCurvedPulses->at(laneIdx).insert(y + ry);
							}
						}
					}
				}
			}

			// レーザーを非カーブのみのグラフへ展開
			for (auto& lane : chartData.note.laser)
			{
				for (auto& [y, section] : lane)
				{
					section = kson::ExpandCurveSegments(section, kson::kCurveSubdivisionInterval);
				}
			}

			// scroll_speedを非カーブのみのグラフへ展開
			chartData.beat.scrollSpeed = kson::ExpandCurveSegments(chartData.beat.scrollSpeed, kson::kCurveSubdivisionInterval);

			// stopをscroll_speedへ焼き込む
			chartData.beat.scrollSpeed = kson::BakeStopIntoScrollSpeed(chartData.beat.scrollSpeed, chartData.beat.stop);

			// 再生速度に応じてBPMをスケーリング
			const double playbackSpeed = createInfo.playOption.playbackSpeed;
			if (playbackSpeed != 1.0)
			{
				for (auto& [pulse, bpm] : chartData.beat.bpm)
				{
					bpm *= playbackSpeed;
				}
			}

			return chartData;
		}
	}

	void GameMain::updateStatus()
	{
		// 曲の音声の更新
		m_bgm.update();

		// 再生時間と現在のBPMを取得
		// TODO: SecondsFに統一
		const double currentTimeSec = m_bgm.posSec().count();
		const double inputDelaySec = m_playOption.effectiveInputDelayMs() / 1000.0;
		const double laserInputDelaySec = m_playOption.effectiveLaserInputDelayMs() / 1000.0;
		const double audioProcDelaySec = m_playOption.effectiveAudioProcDelayMs() / 1000.0;
		const double timingAdjustSec = m_judgmentMain.timingAdjustOffsetSec();
		const double currentTimeSecForButtonJudgment = currentTimeSec - inputDelaySec + timingAdjustSec;
		const double currentTimeSecForLaserJudgment = currentTimeSec - inputDelaySec - laserInputDelaySec + timingAdjustSec;
		const double currentTimeSecForAudioProc = currentTimeSec - audioProcDelaySec;
		const kson::Pulse currentPulse = kson::SecToPulse(currentTimeSec, m_chartData.beat, m_timingCache);
		const double currentPulseDouble = kson::SecToPulseDouble(currentTimeSec, m_chartData.beat, m_timingCache);
		const kson::Pulse currentPulseForButtonJudgment = kson::SecToPulse(currentTimeSecForButtonJudgment, m_chartData.beat, m_timingCache);
		const kson::Pulse currentPulseForLaserJudgment = kson::SecToPulse(currentTimeSecForLaserJudgment, m_chartData.beat, m_timingCache);
		const double currentBPM = kson::TempoAt(currentPulse, m_chartData.beat);
		m_gameStatus.currentTimeSec = currentTimeSec;
		m_gameStatus.currentTimeSecForButtonJudgment = currentTimeSecForButtonJudgment;
		m_gameStatus.currentTimeSecForLaserJudgment = currentTimeSecForLaserJudgment;
		m_gameStatus.currentTimeSecForAudioProc = currentTimeSecForAudioProc;
		m_gameStatus.currentPulse = currentPulse;
		m_gameStatus.currentPulseDouble = currentPulseDouble;
		m_gameStatus.currentPulseForButtonJudgment = currentPulseForButtonJudgment;
		m_gameStatus.currentPulseForLaserJudgment = currentPulseForLaserJudgment;
		m_gameStatus.currentBPM = currentBPM;

		// 視点変更を更新
		// (CamStatusにノーツイベントによる値が相対的に反映されるので、判定の更新より先に実行する必要がある)
		m_camSystem.update(m_chartData, m_gameStatus.currentPulse);
		m_viewStatus.camStatus = m_camSystem.status();

		// 傾きを更新
		m_highwayTilt.update(m_chartData, m_gameStatus.currentPulse);
		m_viewStatus.tiltRadians = m_highwayTilt.radians();
		m_viewStatus.tiltRadiansForBgLayer = m_highwayTilt.radiansForBgLayer();

		// 判定の更新
		m_judgmentMain.update(m_chartData, m_gameStatus, m_viewStatus);

		m_viewStatus.timingAdjustMs = timingAdjustOffsetMs();

		// HARDゲージ/コースモード落ち判定
		if (!m_gameStatus.playFinishStatus.has_value() &&
			(m_playOption.gaugeType == GaugeType::kHardGauge || m_playOption.gameMode == GameMode::kCourseMode) &&
			m_viewStatus.gaugePercentageInt <= kGaugePercentageThresholdHard)
		{
			m_gameStatus.playFinishStatus = PlayFinishStatus
			{
				.finishTimeSec = currentTimeSec,
				.achievement = Achievement::kNone,
				.isHardFailed = IsHardFailedYN::Yes,
			};

			// HARD落ち効果音を再生
			m_hardFailedSound.play();

			// HARD落ち以降の入力は無視
			m_judgmentMain.lockForExit();
		}

		if (!m_gameStatus.playFinishStatus.has_value() && m_judgmentMain.isFinished())
		{
			m_gameStatus.playFinishStatus = PlayFinishStatus
			{
				.finishTimeSec = currentTimeSec,
				.achievement = m_judgmentMain.playResult(m_chartData, m_timingCache, currentTimeSec, IsHardFailedYN::No, false).achievement(),
			};
		}
	}

	void GameMain::updateHighwayScroll()
	{
		// ハイスピードを更新
		if (m_isFirstUpdate)
		{
			// ハイスピードメニュー更新前にHighwayScrollを更新してBPMを設定
			m_highwayScroll.update(m_hispeedSettingMenu.hispeedSetting(), m_gameStatus.currentBPM);
		}
		m_hispeedSettingMenu.update(m_gameStatus.currentBPM);
		m_highwayScroll.update(m_hispeedSettingMenu.hispeedSetting(), m_gameStatus.currentBPM);
	}

	GameMain::GameMain(const GameCreateInfo& createInfo)
		: m_chartFilePath(createInfo.chartFilePath)
		, m_parentPath(FileSystem::ParentPath(createInfo.chartFilePath))
		, m_chartData(LoadChartDataWithTurn(createInfo, &m_laserCurvedPulses))
		, m_timingCache(kson::CreateTimingCache(m_chartData.beat))
		, m_playOption(createInfo.playOption)
		, m_judgmentMain(
			m_chartData,
			m_timingCache,
			createInfo.playOption,
			createInfo.courseContinuation,
			createInfo.playOption.gameMode)
		, m_camSystem(m_chartData)
		, m_highwayScroll(m_chartData)
		, m_bgm(CreateBGM(m_chartData, m_parentPath, createInfo.playOption, createInfo.folderConfIni))
		, m_assistTick(createInfo.assistTickMode)
		, m_laserSlamSE(m_chartData, m_timingCache, m_parentPath, createInfo.playOption.isAutoPlaySE, createInfo.folderConfIni.volumeScale)
		, m_fxChipSE(m_chartData, m_timingCache, m_parentPath, createInfo.playOption.isAutoPlaySE, createInfo.folderConfIni.volumeScale)
		, m_hardFailedSound("se/play_hardfailed.wav")
		, m_audioEffectMain(m_bgm, m_chartData, m_timingCache, m_parentPath, createInfo.playOption.effectiveAudioProcDelayMs() / 1000.0, m_chartData.audio.bgm.vol * createInfo.folderConfIni.volumeScale)
		, m_hispeedSettingMenu(createInfo.playOption.availableHispeedTypes, createInfo.playOption.hispeedSetting, kson::GetEffectiveStdBPM(m_chartData), GetInitialBPM(m_chartData))
		, m_graphicsMain(m_chartData, m_timingCache, m_parentPath, createInfo.playOption)
	{
	}

	void GameMain::start()
	{
		const double globalOffsetSec = m_playOption.effectiveGlobalOffsetMs() / 1000.0;
		m_graphicsMain.prepareMovie(globalOffsetSec);

		if (m_playOption.isTestPlayWithStartMeasure())
		{
			const int32 startMeasure = *m_playOption.testPlayOption->startMeasure;
			const kson::Pulse startPulse = kson::MeasureIdxToPulse(
				startMeasure, m_chartData.beat, m_timingCache);
			const double startSec = kson::PulseToSec(
				startPulse, m_chartData.beat, m_timingCache);

			// 少し手前から再生開始
			const double seekSec = Max(startSec - kTestPlaySeekMarginSec, 0.0);
			m_bgm.seekPosSec(SecondsF{ seekSec });
			m_graphicsMain.seekMoviePosSec(SecondsF{ seekSec });
			m_bgm.play();
		}
		else
		{
			m_bgm.seekPosSec(-TimeSecBeforeStart(m_graphicsMain.hasMovie()));
			m_bgm.play();
		}
	}

	GameMain::StartFadeOutYN GameMain::update()
	{
		// 一時停止・早送りの制御
		processPlaybackControl();

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
		}, m_gameStatus.currentPulse);

		// 効果音の更新
		// TODO: SecondsFに統一
		const double currentTimeSec = m_bgm.posSec().count();
		const double currentTimeSecForAssistTick = currentTimeSec - m_playOption.visualOffsetMs / 1000.0;
		m_assistTick.update(m_chartData, m_timingCache, currentTimeSecForAssistTick);
		m_laserSlamSE.update(m_chartData, m_gameStatus);
		m_fxChipSE.update(m_chartData, m_gameStatus);

		// グラフィックの更新
		m_graphicsMain.update(m_gameStatus, m_viewStatus, m_timingCache);

		m_isFirstUpdate = false;

		return ShouldStartFadeOut(m_gameStatus) ? StartFadeOutYN::Yes : StartFadeOutYN::No;
	}

	void GameMain::draw() const
	{
		// HighwayScrollのコンテキスト
		// (HighwayScrollからの座標取得の引数を省略するためのもの)
		const Scroll::HighwayScrollContext highwayScrollContext(&m_highwayScroll, &m_chartData.beat, &m_timingCache, &m_gameStatus);

		// 描画実行
		m_graphicsMain.draw(m_chartData, m_laserCurvedPulses, m_timingCache, m_gameStatus, m_viewStatus, highwayScrollContext, m_bgm.duration());
	}

	void GameMain::lockForExit()
	{
		m_isAborted = !m_judgmentMain.isFinished();
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
		const IsHardFailedYN isHardFailed{ m_gameStatus.playFinishStatus.has_value() && m_gameStatus.playFinishStatus->isHardFailed };
		return m_judgmentMain.playResult(m_chartData, m_timingCache, m_gameStatus.currentTimeSec, isHardFailed, m_isAborted);
	}

	int32 GameMain::timingAdjustOffsetMs() const
	{
		return static_cast<int32>(Round(-m_judgmentMain.timingAdjustOffsetSec() * 1000.0));
	}

	void GameMain::startBGMFadeOut(Duration duration)
	{
		m_bgm.setFadeOut(duration);
	}

	void GameMain::processPlaybackControl()
	{
		const bool isCtrlPressed = PlatformKey::KeyCommandControl.pressed();
		const bool isAltPressed = KeyAlt.pressed();

		// 一時停止/再開(Ctrl+Enter)
		if (isCtrlPressed && KeyEnter.down())
		{
			if (m_isPaused)
			{
				m_bgm.play();
				m_isPaused = false;
			}
			else
			{
				m_bgm.pause();
				m_isPaused = true;
			}
			m_gameStatus.isPaused = m_isPaused;
		}

		// 早送り(Ctrl+Right)
		if (!m_isPaused && isCtrlPressed && !isAltPressed && KeyRight.pressed())
		{
			// 押した瞬間は即時シーク、長押し中は一定間隔でシーク
			if (KeyRight.down() || m_fastForwardStopwatch.ms() >= 60)
			{
				const auto currentPos = m_bgm.posSec();
				const auto newPos = currentPos + SecondsF{ 1.0 };
				m_bgm.seekPosSec(newPos);
				m_graphicsMain.seekMoviePosSec(newPos);
				m_fastForwardStopwatch.restart();
			}
		}
		else
		{
			m_fastForwardStopwatch.restart();
		}

		// 手動タイミング調整(Ctrl+Alt+矢印で±5ms、Ctrl+Alt+Shift+矢印で±1ms)
		if (isCtrlPressed && isAltPressed)
		{
			if (m_syncAdjustStopwatch.ms() >= 120)
			{
				const bool isShiftPressed = KeyShift.pressed();
				const double stepMs = isShiftPressed ? 1.0 : 5.0;
				if (KeyRight.pressed())
				{
					m_judgmentMain.addTimingAdjustOffset(stepMs / 1000.0);
					m_syncAdjustStopwatch.restart();
				}
				else if (KeyLeft.pressed())
				{
					m_judgmentMain.addTimingAdjustOffset(-stepMs / 1000.0);
					m_syncAdjustStopwatch.restart();
				}
			}
		}
		else
		{
			m_syncAdjustStopwatch.restart();
		}
	}
}
