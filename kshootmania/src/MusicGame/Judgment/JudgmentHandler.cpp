#include "JudgmentHandler.hpp"
#include "ButtonLaneJudgment.hpp"
#include "LaserLaneJudgment.hpp"

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

		int32 TotalCombo(const BTLaneJudgments& btLaneJudgments, const FXLaneJudgments& fxLaneJudgments, const LaserLaneJudgments& laserLaneJudgments)
		{
			const int32 btCombo = ApplyAndSum(btLaneJudgments, [](const ButtonLaneJudgment& laneJudgment) { return static_cast<int32>(laneJudgment.chipJudgmentCount() + laneJudgment.longJudgmentCount()); });
			const int32 fxCombo = ApplyAndSum(fxLaneJudgments, [](const ButtonLaneJudgment& laneJudgment) { return static_cast<int32>(laneJudgment.chipJudgmentCount() + laneJudgment.longJudgmentCount()); });
			const int32 laserCombo = ApplyAndSum(laserLaneJudgments, [](const LaserLaneJudgment& laneJudgment) { return static_cast<int32>(laneJudgment.lineJudgmentCount() + laneJudgment.slamJudgmentCount()); });
			return btCombo + fxCombo + laserCombo;
		}

		int32 TotalGaugeValueButtonLane(ButtonLaneJudgment laneJudgment, int32 chipValue, int32 longValue)
		{
			const int32 chipJudgmentCount = static_cast<int32>(laneJudgment.chipJudgmentCount());
			const int32 longJudgmentCount = static_cast<int32>(laneJudgment.longJudgmentCount());
			return chipJudgmentCount * chipValue + longJudgmentCount * longValue;
		}

		int32 TotalGaugeValueLaserLane(LaserLaneJudgment laneJudgment, int32 chipValue, int32 longValue)
		{
			const int32 lineJudgmentCount = static_cast<int32>(laneJudgment.lineJudgmentCount());
			const int32 slamJudgmentCount = static_cast<int32>(laneJudgment.slamJudgmentCount());
			return lineJudgmentCount * longValue + slamJudgmentCount * chipValue;
		}

		int32 TotalGaugeValue(
			const std::array<ButtonLaneJudgment, kson::kNumBTLanesSZ>& btLaneJudgments,
			const std::array<ButtonLaneJudgment, kson::kNumFXLanesSZ>& fxLaneJudgments,
			const std::array<LaserLaneJudgment, kson::kNumLaserLanesSZ>& laserLaneJudgments,
			int32 chipValue,
			int32 longValue)
		{
			const auto totalGaugeValueButtonLane = [chipValue, longValue](ButtonLaneJudgment laneJudgment)
			{
				return TotalGaugeValueButtonLane(laneJudgment, chipValue, longValue);
			};

			const auto totalGaugeValueLaserLane = [chipValue, longValue](LaserLaneJudgment laneJudgment)
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
			const std::array<ButtonLaneJudgment, kson::kNumBTLanesSZ>& btLaneJudgments,
			const std::array<ButtonLaneJudgment, kson::kNumFXLanesSZ>& fxLaneJudgments,
			const std::array<LaserLaneJudgment, kson::kNumLaserLanesSZ>& laserLaneJudgments)
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

	JudgmentHandler::JudgmentHandler(const kson::ChartData& chartData, const BTLaneJudgments& btLaneJudgments, const FXLaneJudgments& fxLaneJudgments, const LaserLaneJudgments& laserLaneJudgments, const PlayOption& playOption, const Optional<CourseContinuation>& courseContinuation, GameMode gameMode)
		: m_playOption(playOption)
		, m_totalCombo(TotalCombo(btLaneJudgments, fxLaneJudgments, laserLaneJudgments))
		, m_scoringStatus(
			TotalGaugeValue(btLaneJudgments, fxLaneJudgments, laserLaneJudgments, kScoreValueCritical, kScoreValueCritical),
			GaugeValueMax(chartData.gauge.total, btLaneJudgments, fxLaneJudgments, laserLaneJudgments),
			playOption.gaugeType,
			courseContinuation,
			gameMode)
		, m_camPatternMain(chartData)
	{
	}

	void JudgmentHandler::onChipJudged(JudgmentResult result)
	{
		assert(result == JudgmentResult::kCritical || result == JudgmentResult::kNearFast || result == JudgmentResult::kNearSlow || result == JudgmentResult::kError);

		if (m_isLockedForExit)
		{
			return;
		}

		m_scoringStatus.onChipOrLaserSlamJudgment(result);
	}

	void JudgmentHandler::onLongJudged(JudgmentResult result)
	{
		assert(result == JudgmentResult::kCritical || result == JudgmentResult::kError);

		if (m_isLockedForExit)
		{
			return;
		}

		m_scoringStatus.onLongOrLaserLineJudgment(result);
	}

	void JudgmentHandler::onLaserLineJudged(JudgmentResult result)
	{
		assert(result == JudgmentResult::kCritical || result == JudgmentResult::kError);

		if (m_isLockedForExit)
		{
			return;
		}

		m_scoringStatus.onLongOrLaserLineJudgment(result);
	}

	void JudgmentHandler::onLaserSlamJudged(JudgmentResult result, kson::Pulse laserSlamPulse, double prevTimeSec, kson::Pulse prevPulse, int32 direction)
	{
		assert(result == JudgmentResult::kCritical || result == JudgmentResult::kError);

		if (m_isLockedForExit)
		{
			return;
		}

		m_scoringStatus.onChipOrLaserSlamJudgment(result);

		if (result != JudgmentResult::kError)
		{
			if (!m_camPatternMain.hasSwingEvent(laserSlamPulse, direction))
			{
				m_laserSlamShakeStatus.onLaserSlamJudged(prevTimeSec, direction);
			}
			m_camPatternMain.onLaserSlamJudged(laserSlamPulse, direction, prevPulse);
		}
	}

	void JudgmentHandler::applyToViewStatus(ViewStatus& viewStatusRef, double currentTimeSec, kson::Pulse currentPulse)
	{
		// ScoringStatusをViewStatusに反映
		viewStatusRef.score = m_scoringStatus.score();
		viewStatusRef.gaugePercentage = m_scoringStatus.gaugePercentage(m_playOption.gaugeType);
		viewStatusRef.gaugePercentageInt = m_scoringStatus.gaugePercentageInt(m_playOption.gaugeType);
		const int32 displayCombo = m_scoringStatus.displayCombo();
		viewStatusRef.displayCombo = displayCombo;
		viewStatusRef.displayIsNoError = m_scoringStatus.displayIsNoError();

		// 100コンボ毎にアニメーションの開始時刻を記録する
		if (displayCombo > m_prevDisplayCombo && displayCombo / 100 > m_prevDisplayCombo / 100)
		{
			m_comboMilestoneEffectStartTimeSec = currentTimeSec;
		}
		m_prevDisplayCombo = displayCombo;
		viewStatusRef.comboMilestoneEffectStartTimeSec = m_comboMilestoneEffectStartTimeSec;

		// 直角LASERの振動をViewStatusに反映
		m_laserSlamShakeStatus.applyToCamStatus(viewStatusRef.camStatus, currentTimeSec);

		// 視点変更パターン(回転など)をViewStatusに反映
		m_camPatternMain.applyToCamStatus(viewStatusRef.camStatus, currentPulse);
	}

	void JudgmentHandler::lockForExit()
	{
		m_isLockedForExit = true;
	}

	bool JudgmentHandler::isFinished() const
	{
		return m_totalCombo <= m_scoringStatus.totalJudgedCombo();
	}

	double JudgmentHandler::timingAdjustOffsetSec() const
	{
		return m_timingAdjustOffsetSec;
	}

	void JudgmentHandler::applyAutoSyncTimingAdjust(double diffSec)
	{
		const int32 autoSyncLevel = static_cast<int32>(m_playOption.autoSyncMode);
		if (autoSyncLevel > 0)
		{
			m_timingAdjustOffsetSec += diffSec * autoSyncLevel / 90.0;
		}
	}

	void JudgmentHandler::addTimingAdjustOffset(double offsetSec)
	{
		m_timingAdjustOffsetSec += offsetSec;
	}

	PlayResult JudgmentHandler::playResult(double currentTimeSec, double chartEndTimeSec, IsHardFailedYN isHardFailed, bool isAborted) const
	{
		const double chartTimeProgress = chartEndTimeSec > 0.0 ? Clamp(currentTimeSec / chartEndTimeSec, 0.0, 1.0) : 1.0;

		return PlayResult
		{
			.score = m_scoringStatus.score(),
			.maxCombo = m_scoringStatus.maxCombo(),
			.finalCourseCombo = m_scoringStatus.courseCombo(),
			.totalCombo = m_totalCombo,
			.comboStats = m_scoringStatus.comboStats(),
			.playOption = m_playOption,
			.gaugePercentage = m_scoringStatus.gaugePercentage(m_playOption.gaugeType),
			.gaugePercentageForGrade = m_scoringStatus.gaugePercentageForGrade(),
			.gaugeValue = m_scoringStatus.gaugeValue(),
			.chartTimeProgress = chartTimeProgress,
			.isHardFailed = isHardFailed,
			.isAborted = isAborted,
		};
	}
}
