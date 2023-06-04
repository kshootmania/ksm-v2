#include "judgment_main.hpp"

namespace MusicGame::Judgment
{
	JudgmentMain::JudgmentMain(const kson::ChartData& chartData, const kson::TimingCache& timingCache)
		: m_btLaneJudgments{
			ButtonLaneJudgment(kBTButtons[0], chartData.note.bt[0], chartData.beat, timingCache),
			ButtonLaneJudgment(kBTButtons[1], chartData.note.bt[1], chartData.beat, timingCache),
			ButtonLaneJudgment(kBTButtons[2], chartData.note.bt[2], chartData.beat, timingCache),
			ButtonLaneJudgment(kBTButtons[3], chartData.note.bt[3], chartData.beat, timingCache) }
		, m_fxLaneJudgments{
			ButtonLaneJudgment(kFXButtons[0], chartData.note.fx[0], chartData.beat, timingCache),
			ButtonLaneJudgment(kFXButtons[1], chartData.note.fx[1], chartData.beat, timingCache) }
		, m_laserLaneJudgments{
			LaserLaneJudgment(kLaserButtons[0][0], kLaserButtons[0][1], chartData.note.laser[0], chartData.beat, timingCache),
			LaserLaneJudgment(kLaserButtons[1][0], kLaserButtons[1][1], chartData.note.laser[1], chartData.beat, timingCache) }
		, m_judgmentHandler(chartData, m_btLaneJudgments, m_fxLaneJudgments, m_laserLaneJudgments)
	{
	}

	void JudgmentMain::update(const kson::ChartData& chartData, GameStatus& gameStatusRef, ViewStatus& viewStatusRef)
	{
		// BTレーンの判定
		for (std::size_t i = 0U; i < kson::kNumBTLanesSZ; ++i)
		{
			m_btLaneJudgments[i].update(chartData.note.bt[i], gameStatusRef.currentPulse, gameStatusRef.currentTimeSec, gameStatusRef.btLaneStatus[i], m_judgmentHandler);
		}

		// FXレーンの判定
		for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
		{
			m_fxLaneJudgments[i].update(chartData.note.fx[i], gameStatusRef.currentPulse, gameStatusRef.currentTimeSec, gameStatusRef.fxLaneStatus[i], m_judgmentHandler);
		}

		// LASERレーンの判定
		for (std::size_t i = 0U; i < kson::kNumLaserLanesSZ; ++i)
		{
			m_laserLaneJudgments[i].update(chartData.note.laser[i], gameStatusRef.currentPulse, gameStatusRef.currentTimeSec, gameStatusRef.laserLaneStatus[i], m_judgmentHandler);
		}

		// 状態をViewStatusに反映
		m_judgmentHandler.applyToViewStatus(viewStatusRef, gameStatusRef.currentTimeSec, gameStatusRef.currentPulse);
	}
}
