#include "judgment_main.hpp"

namespace MusicGame::Judgment
{
	namespace
	{
		template <typename Container, typename Func>
		int32 ApplyAndSum(const Container& container, Func func)
		{
			int32 sum = 0;
			for (const auto& item : container)
			{
				sum += func(item);
			}
			return sum;
		}

		int32 TotalGaugeValueButtonLane(Judgment::ButtonLaneJudgment laneJudgment, int32 chipValue, int32 longValue)
		{
			const int32 chipJudgmentCount = static_cast<int32>(laneJudgment.chipJudgmentCount());
			const int32 longJudgmentCount = static_cast<int32>(laneJudgment.longJudgmentCount());
			return chipJudgmentCount * chipValue + longJudgmentCount * longValue;
		}

		int32 TotalGaugeValueLaserLane(Judgment::LaserLaneJudgment laneJudgment, int32 chipValue, int32 longValue)
		{
			const int32 lineJudgmentCount = static_cast<int32>(laneJudgment.lineJudgmentCount());
			const int32 slamJudgmentCount = static_cast<int32>(laneJudgment.slamJudgmentCount());
			return lineJudgmentCount * longValue + slamJudgmentCount * chipValue;
		}

		int32 TotalGaugeValue(
			const std::array<Judgment::ButtonLaneJudgment, kson::kNumBTLanesSZ>& btLaneJudgments,
			const std::array<Judgment::ButtonLaneJudgment, kson::kNumFXLanesSZ>& fxLaneJudgments,
			const std::array<Judgment::LaserLaneJudgment, kson::kNumLaserLanesSZ>& laserLaneJudgments,
			int32 chipValue,
			int32 longValue)
		{
			const auto totalGaugeValueButtonLane = [chipValue, longValue](Judgment::ButtonLaneJudgment laneJudgment)
			{
				return TotalGaugeValueButtonLane(laneJudgment, chipValue, longValue);
			};

			const auto totalGaugeValueLaserLane = [chipValue, longValue](Judgment::LaserLaneJudgment laneJudgment)
			{
				return TotalGaugeValueLaserLane(laneJudgment, chipValue, longValue);
			};

			return
				ApplyAndSum(btLaneJudgments, totalGaugeValueButtonLane) +
				ApplyAndSum(fxLaneJudgments, totalGaugeValueButtonLane) +
				ApplyAndSum(laserLaneJudgments, totalGaugeValueLaserLane);
		}

		int32 GaugeValueMax(
			int32 total,
			const std::array<Judgment::ButtonLaneJudgment, kson::kNumBTLanesSZ>& btLaneJudgments,
			const std::array<Judgment::ButtonLaneJudgment, kson::kNumFXLanesSZ>& fxLaneJudgments,
			const std::array<Judgment::LaserLaneJudgment, kson::kNumLaserLanesSZ>& laserLaneJudgments)
		{
			// HSP版: https://github.com/kshootmania/ksm-v1/blob/8deaf1fd147f6e13ac6665731e1ff1e00c5b4176/src/scene/play/play_init.hsp#L238-L252

			const int32 totalGaugeValue = TotalGaugeValue(btLaneJudgments, fxLaneJudgments, laserLaneJudgments, kGaugeValueChip, kGaugeValueLong);

			if (total >= 100)
			{
				// 譜面にTOTAL値が指定されている場合、それをもとに決定
				return Max(totalGaugeValue * 100 / total, 1);
			}

			// TOTAL値が指定されていない場合、チップノーツの割合が多いほどゲージが重くなるような方式で自動的に決定
			constexpr int32 kGaugeValueMaxFactorChip = 120;
			constexpr int32 kGaugeValueMaxFactorLong = 20;
			int32 gaugeValueMax = TotalGaugeValue(btLaneJudgments, fxLaneJudgments, laserLaneJudgments, kGaugeValueMaxFactorChip, kGaugeValueMaxFactorLong);
			if (gaugeValueMax < 125000)
			{
				gaugeValueMax = gaugeValueMax * gaugeValueMax / 125000 / 5 + gaugeValueMax * 4 / 5;
			}
			if (gaugeValueMax > 100000)
			{
				gaugeValueMax = 100000 + (gaugeValueMax - 100000) / 2;
			}
			gaugeValueMax = Min(gaugeValueMax, 125000);
			return Clamp(gaugeValueMax, 1, totalGaugeValue);
		}
	}

	JudgmentMain::JudgmentMain(const kson::ChartData& chartData, const kson::TimingCache& timingCache)
		: m_btLaneJudgments{
			Judgment::ButtonLaneJudgment(kBTButtons[0], chartData.note.bt[0], chartData.beat, timingCache),
			Judgment::ButtonLaneJudgment(kBTButtons[1], chartData.note.bt[1], chartData.beat, timingCache),
			Judgment::ButtonLaneJudgment(kBTButtons[2], chartData.note.bt[2], chartData.beat, timingCache),
			Judgment::ButtonLaneJudgment(kBTButtons[3], chartData.note.bt[3], chartData.beat, timingCache) }
		, m_fxLaneJudgments{
			Judgment::ButtonLaneJudgment(kFXButtons[0], chartData.note.fx[0], chartData.beat, timingCache),
			Judgment::ButtonLaneJudgment(kFXButtons[1], chartData.note.fx[1], chartData.beat, timingCache) }
		, m_laserLaneJudgments{
			Judgment::LaserLaneJudgment(kLaserButtons[0][0], kLaserButtons[0][1], chartData.note.laser[0], chartData.beat, timingCache),
			Judgment::LaserLaneJudgment(kLaserButtons[1][0], kLaserButtons[1][1], chartData.note.laser[1], chartData.beat, timingCache) }
		, m_scoringStatus(
			TotalGaugeValue(m_btLaneJudgments, m_fxLaneJudgments, m_laserLaneJudgments, Judgment::kScoreValueCritical, Judgment::kScoreValueCritical),
			GaugeValueMax(chartData.gauge.total, m_btLaneJudgments, m_fxLaneJudgments, m_laserLaneJudgments))
	{
	}

	void JudgmentMain::update(const kson::ChartData& chartData, GameStatus& gameStatusRef, ViewStatus& viewStatusRef)
	{
		// BTレーンの判定
		for (std::size_t i = 0U; i < kson::kNumBTLanesSZ; ++i)
		{
			m_btLaneJudgments[i].update(chartData.note.bt[i], gameStatusRef.currentPulse, gameStatusRef.currentTimeSec, gameStatusRef.btLaneStatus[i], m_scoringStatus);
		}

		// FXレーンの判定
		for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
		{
			m_fxLaneJudgments[i].update(chartData.note.fx[i], gameStatusRef.currentPulse, gameStatusRef.currentTimeSec, gameStatusRef.fxLaneStatus[i], m_scoringStatus);
		}

		// LASERレーンの判定
		for (std::size_t i = 0U; i < kson::kNumLaserLanesSZ; ++i)
		{
			m_laserLaneJudgments[i].update(chartData.note.laser[i], gameStatusRef.currentPulse, gameStatusRef.currentTimeSec, gameStatusRef.laserLaneStatus[i], m_scoringStatus, viewStatusRef.laserSlamWiggleStatus);
		}

		// ScoringStatusをViewStatusに反映
		viewStatusRef.score = m_scoringStatus.score();
		viewStatusRef.gaugePercentage = m_scoringStatus.gaugePercentage();
		viewStatusRef.combo = m_scoringStatus.combo();
		viewStatusRef.isNoError = m_scoringStatus.isNoError();
	}
}
