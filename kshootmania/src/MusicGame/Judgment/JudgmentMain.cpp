#include "JudgmentMain.hpp"
#include "kson/Util/TimingUtils.hpp"

namespace MusicGame::Judgment
{

	JudgmentMain::JudgmentMain(const kson::ChartData& chartData, const kson::TimingCache& timingCache, const PlayOption& playOption, const Optional<CourseContinuation>& courseContinuation, GameMode gameMode)
		: m_playOption(playOption)
		, m_btLaneJudgments{
			ButtonLaneJudgment(playOption.effectiveBtJudgmentPlayMode(), playOption.gaugeType, playOption.fastSlowMode, kButtonBT_A, chartData.note.bt[0], chartData.beat, timingCache),
			ButtonLaneJudgment(playOption.effectiveBtJudgmentPlayMode(), playOption.gaugeType, playOption.fastSlowMode, kButtonBT_B, chartData.note.bt[1], chartData.beat, timingCache),
			ButtonLaneJudgment(playOption.effectiveBtJudgmentPlayMode(), playOption.gaugeType, playOption.fastSlowMode, kButtonBT_C, chartData.note.bt[2], chartData.beat, timingCache),
			ButtonLaneJudgment(playOption.effectiveBtJudgmentPlayMode(), playOption.gaugeType, playOption.fastSlowMode, kButtonBT_D, chartData.note.bt[3], chartData.beat, timingCache) }
		, m_fxLaneJudgments{
			ButtonLaneJudgment(playOption.effectiveFxJudgmentPlayMode(), playOption.gaugeType, playOption.fastSlowMode, kButtonFX_L, chartData.note.fx[0], chartData.beat, timingCache),
			ButtonLaneJudgment(playOption.effectiveFxJudgmentPlayMode(), playOption.gaugeType, playOption.fastSlowMode, kButtonFX_R, chartData.note.fx[1], chartData.beat, timingCache) }
		, m_laserLaneJudgments{
			LaserLaneJudgment(playOption.effectiveLaserJudgmentPlayMode(), 0, kButtonLeftLaserL, kButtonLeftLaserR, chartData.note.laser[0], chartData.beat, timingCache),
			LaserLaneJudgment(playOption.effectiveLaserJudgmentPlayMode(), 1, kButtonRightLaserL, kButtonRightLaserR, chartData.note.laser[1], chartData.beat, timingCache) }
		, m_judgmentHandler(chartData, m_btLaneJudgments, m_fxLaneJudgments, m_laserLaneJudgments, playOption, courseContinuation, gameMode)
	{
	}

	void JudgmentMain::update(const kson::ChartData& chartData, GameStatus& gameStatusRef, ViewStatus& viewStatusRef)
	{
		// BTレーンの判定
		for (std::size_t i = 0U; i < kson::kNumBTLanesSZ; ++i)
		{
			m_btLaneJudgments[i].update(chartData, chartData.note.bt[i], gameStatusRef.currentPulseForButtonJudgment, gameStatusRef.currentTimeSecForButtonJudgment, gameStatusRef.currentTimeSec, gameStatusRef.btLaneStatus[i], m_judgmentHandler);
		}

		// FXレーンの判定
		for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
		{
			m_fxLaneJudgments[i].update(chartData, chartData.note.fx[i], gameStatusRef.currentPulseForButtonJudgment, gameStatusRef.currentTimeSecForButtonJudgment, gameStatusRef.currentTimeSec, gameStatusRef.fxLaneStatus[i], m_judgmentHandler);
		}

		// LASERレーンの判定
		for (std::size_t i = 0U; i < kson::kNumLaserLanesSZ; ++i)
		{
			m_laserLaneJudgments[i].update(chartData.note.laser[i], gameStatusRef.currentPulseForLaserJudgment, gameStatusRef.currentPulse, gameStatusRef.currentTimeSecForLaserJudgment, gameStatusRef.currentTimeSec, gameStatusRef.laserLaneStatus[i], m_judgmentHandler);
		}

		// 状態をViewStatusに反映
		m_judgmentHandler.applyToViewStatus(viewStatusRef, gameStatusRef.currentTimeSec, gameStatusRef.currentPulse);
	}

	void JudgmentMain::lockForExit()
	{
		// ERROR判定通知時にゲージが変動しないよう先にロック
		m_judgmentHandler.lockForExit();

		// 残りの未判定ノーツを全てERROR判定にする
		for (std::size_t i = 0U; i < kson::kNumBTLanesSZ; ++i)
		{
			m_btLaneJudgments[i].lockForExit(m_judgmentHandler);
		}
		for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
		{
			m_fxLaneJudgments[i].lockForExit(m_judgmentHandler);
		}
		for (std::size_t i = 0U; i < kson::kNumLaserLanesSZ; ++i)
		{
			m_laserLaneJudgments[i].lockForExit(m_judgmentHandler);
		}
	}

	PlayResult JudgmentMain::playResult(const kson::ChartData& chartData, const kson::TimingCache& timingCache, double currentTimeSec, IsHardFailedYN isHardFailed, bool isAborted) const
	{
		const kson::Pulse lastNoteEndY = kson::LastNoteEndY(chartData.note);
		const double chartEndTimeSec = kson::PulseToSec(lastNoteEndY, chartData.beat, timingCache);
		return m_judgmentHandler.playResult(currentTimeSec, chartEndTimeSec, isHardFailed, isAborted);
	}

	bool JudgmentMain::isFinished() const
	{
		return m_judgmentHandler.isFinished();
	}
}
