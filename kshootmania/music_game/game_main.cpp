#include "game_main.hpp"
#include "game_defines.hpp"
#include "kson/io/ksh_io.hpp"
#include "kson/io/kson_io.hpp"

namespace MusicGame
{
	namespace
	{
		template <class LaneJudgment, std::size_t N>
		int32 SumScoreFactor(const std::array<LaneJudgment, N>& laneJudgements)
		{
			int32 sum = 0;
			for (const auto& laneJudgment : laneJudgements)
			{
				sum += laneJudgment.scoreValue();
			}
			return sum;
		}

		template <class LaneJudgment, std::size_t N>
		int32 SumScoreFactorMax(const std::array<LaneJudgment, N>& laneJudgements)
		{
			int32 sum = 0;
			for (const auto& laneJudgment : laneJudgements)
			{
				sum += laneJudgment.scoreValueMax();
			}
			return sum;
		}
	}

	void GameMain::updateGameStatus()
	{
		const double currentTimeSec = m_bgm.posSec();
		const kson::Pulse currentPulse = kson::SecToPulse(currentTimeSec, m_chartData.beat, m_timingCache);
		const double currentBPM = kson::TempoAt(currentPulse, m_chartData.beat);
		m_gameStatus.currentTimeSec = currentTimeSec;
		m_gameStatus.currentPulse = currentPulse;
		m_gameStatus.currentBPM = currentBPM;

		// BTレーンの判定
		for (std::size_t i = 0U; i < kson::kNumBTLanesSZ; ++i)
		{
			m_btLaneJudgments[i].update(m_chartData.note.bt[i], currentPulse, currentTimeSec, m_gameStatus.btLaneStatus[i]);
		}

		// FXレーンの判定
		for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
		{
			m_fxLaneJudgments[i].update(m_chartData.note.fx[i], currentPulse, currentTimeSec, m_gameStatus.fxLaneStatus[i]);
		}

		// LASERレーンの判定
		for (std::size_t i = 0U; i < kson::kNumLaserLanesSZ; ++i)
		{
			m_laserLaneJudgments[i].update(m_chartData.note.laser[i], currentPulse, currentTimeSec, m_gameStatus.laserLaneStatus[i]);
		}

		m_gameStatus.score = static_cast<int32>(static_cast<int64>(kScoreMax) * (SumScoreFactor(m_btLaneJudgments) + SumScoreFactor(m_fxLaneJudgments) + SumScoreFactor(m_laserLaneJudgments)) / m_scoreFactorMax);

		// TODO: Calculate camera values
	}

	GameMain::GameMain(const GameCreateInfo& gameCreateInfo)
		: m_parentPath(FileSystem::ParentPath(gameCreateInfo.chartFilePath))
		, m_chartData(kson::LoadKSHChartData(gameCreateInfo.chartFilePath.narrow()))
		, m_timingCache(kson::CreateTimingCache(m_chartData.beat))
		, m_btLaneJudgments{
			Judgment::ButtonLaneJudgment(kBTButtons[0], m_chartData.note.bt[0], m_chartData.beat, m_timingCache),
			Judgment::ButtonLaneJudgment(kBTButtons[1], m_chartData.note.bt[1], m_chartData.beat, m_timingCache),
			Judgment::ButtonLaneJudgment(kBTButtons[2], m_chartData.note.bt[2], m_chartData.beat, m_timingCache),
			Judgment::ButtonLaneJudgment(kBTButtons[3], m_chartData.note.bt[3], m_chartData.beat, m_timingCache) }
		, m_fxLaneJudgments{
			Judgment::ButtonLaneJudgment(kFXButtons[0], m_chartData.note.fx[0], m_chartData.beat, m_timingCache),
			Judgment::ButtonLaneJudgment(kFXButtons[1], m_chartData.note.fx[1], m_chartData.beat, m_timingCache) }
		, m_laserLaneJudgments{
			Judgment::LaserLaneJudgment(kLaserButtons[0][0], kLaserButtons[0][1], m_chartData.note.laser[0], m_chartData.beat, m_timingCache),
			Judgment::LaserLaneJudgment(kLaserButtons[1][0], kLaserButtons[1][1], m_chartData.note.laser[1], m_chartData.beat, m_timingCache) }
		, m_scoreFactorMax(SumScoreFactorMax(m_btLaneJudgments) + SumScoreFactorMax(m_fxLaneJudgments) + SumScoreFactorMax(m_laserLaneJudgments))
		, m_bgm(m_parentPath + U"/" + Unicode::FromUTF8(m_chartData.audio.bgm.filename))
		, m_assistTick(gameCreateInfo.enableAssistTick)
		, m_audioEffectMain(m_bgm, m_chartData, m_timingCache)
		, m_graphicsMain(m_chartData, m_parentPath)
	{
		m_bgm.seekPosSec(-TimeSecBeforeStart(false/* TODO: movie */));
		m_bgm.play();

		kson::SaveKSONChartData("hogehoge.kson", m_chartData);
	}

	void GameMain::update()
	{
		m_bgm.update();

		// ゲームステータスの更新とノーツ判定
		updateGameStatus();

		// 音声エフェクトの更新
		std::array<Optional<bool>, kson::kNumFXLanesSZ> longFXPressed;
		for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
		{
			longFXPressed[i] = m_gameStatus.fxLaneStatus[i].longNotePressed;
		}
		m_audioEffectMain.update(m_bgm, m_chartData, m_timingCache, {
			.longFXPressed = longFXPressed,
		});

		// 効果音の更新
		const double currentTimeSec = m_bgm.posSec();
		m_assistTick.update(m_chartData, m_timingCache, currentTimeSec);

		// グラフィックスの更新
		m_graphicsMain.update(m_chartData, m_gameStatus);
	}

	void GameMain::draw() const
	{
		m_graphicsMain.draw(m_chartData, m_gameStatus);
	}
}
