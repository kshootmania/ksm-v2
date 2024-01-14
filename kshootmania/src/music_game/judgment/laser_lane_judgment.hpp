﻿#pragma once
#include "music_game/game_defines.hpp"
#include "music_game/game_status.hpp"
#include "music_game/view_status.hpp"
#include "music_game/judgment/judgment_handler.hpp"
#include "kson/chart_data.hpp"
#include "kson/util/timing_utils.hpp"

namespace MusicGame::Judgment
{
	class LaserSlamJudgment
	{
	private:
		// 時間(秒)
		const double m_sec;

		// 方向(-1:左, +1:右)
		const int32 m_direction;

		// カーソルの累計移動量
		double m_totalAbsDeltaCursorX = 0.0;

	public:
		LaserSlamJudgment(double sec, int32 direction);

		double sec() const;

		int32 direction() const;

		void addDeltaCursorX(double deltaCursorX, double currentTimeSec);

		bool isCriticalSatisfied() const;

		JudgmentResult judgmentResult(double currentTimeSec, IsAutoPlayYN isAutoPlay) const;
	};

	class LaserLaneJudgment
	{
	public:
		struct LineJudgment
		{
			kson::RelPulse length = kson::RelPulse{ 0 };

			JudgmentResult result = JudgmentResult::kUnspecified;
		};

	private:
		const PlayOption m_playOption;
		const JudgmentPlayMode m_judgmentPlayMode;
		const KeyConfig::Button m_keyConfigButtonL;
		const KeyConfig::Button m_keyConfigButtonR;
		const kson::ByPulse<int32> m_laserLineDirectionMap;
		const kson::ByPulse<int32> m_laserLineDirectionMapForRippleEffect;
		const Array<double> m_laserLineDirectionChangeSecArray;

		bool m_isLockedForExit = false;

		Array<double>::const_iterator m_laserLineDirectionChangeSecArrayCursor;

		kson::ByPulse<LineJudgment> m_lineJudgmentArray;
		kson::ByPulse<LineJudgment>::iterator m_passedLineJudgmentCursor;

		kson::ByPulse<LaserSlamJudgment> m_slamJudgmentArray;
		kson::ByPulse<LaserSlamJudgment>::iterator m_slamJudgmentArrayCursor;

		double m_lastCorrectMovementSec = kPastTimeSec;

		Optional<kson::Pulse> m_prevCurrentLaserSectionPulse = none;
		bool m_prevIsCursorInCriticalJudgmentRange = false;
		bool m_prevIsCursorInAutoFitRange = false;
		kson::Pulse m_prevPulse = kPastPulse;
		double m_prevTimeSec = kPastTimeSec;

		void processCursorMovement(double deltaCursorX, kson::Pulse currentPulse, double currentTimeSec, LaserLaneStatus& laneStatusRef);

		void processSlamJudgment(const kson::ByPulse<kson::LaserSection>& lane, double deltaCursorX, double currentTimeSec, LaserLaneStatus& laneStatusRef, JudgmentHandler& judgmentHandlerRef, IsAutoPlayYN isAutoPlay);

		void processAutoCursorMovementBySlamJudgment(double currentTimeSec, LaserLaneStatus& laneStatusRef);

		void processAutoCursorMovementForAutoPlay(double currentTimeSec, LaserLaneStatus& laneStatusRef);

		void processAutoCursorMovementByLineDirectionChange(double currentTimeSec, LaserLaneStatus& laneStatusRef);

		void processAutoCursorMovementAfterCorrectMovement(double currentTimeSec, LaserLaneStatus& laneStatusRef);

		void processLineJudgment(const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse, double currentTimeSec, LaserLaneStatus& laneStatusRef, JudgmentHandler& judgmentHandlerRef);

		void processPassedLineJudgment(const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse, LaserLaneStatus& laneStatusRef, JudgmentHandler& judgmentHandlerRef, IsAutoPlayYN isAutoPlay);

	public:
		LaserLaneJudgment(const PlayOption& playOption, JudgmentPlayMode judgmentPlayMode, KeyConfig::Button keyConfigButtonL, KeyConfig::Button keyConfigButtonR, const kson::ByPulse<kson::LaserSection>& lane, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache);

		void update(const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse, double currentSec, kson::Pulse currentRawPulse, double currentRawSec, LaserLaneStatus& laneStatusRef, JudgmentHandler& judgmentHandlerRef);

		void lockForExit();

		std::size_t lineJudgmentCount() const;

		std::size_t slamJudgmentCount() const;
	};
}
