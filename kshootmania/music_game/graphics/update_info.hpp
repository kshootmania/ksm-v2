#pragma once
#include "music_game/judgment/judgment_defines.hpp"
#include "ksh/common/common.hpp"
#include "ksh/chart_data.hpp"

namespace MusicGame::Graphics
{
	struct LaneState
	{
		double keyBeamTimeSec = kPastTimeSec;
		Judgment::KeyBeamType keyBeamType = Judgment::KeyBeamType::kDefault;

		double animationStartTimeSec = kPastTimeSec;
		Judgment::JudgmentResult animationJudgmentResult = Judgment::JudgmentResult::kUnspecified;
	};

	struct CamValues
	{
		double zoom = 0.0;
		double shiftX = 0.0;
		double rotationX = 0.0;
		double rotationZ = 0.0;
		double rotationZLane = 0.0;
		double rotationZJdgLine = 0.0;
	};

	struct UpdateInfo
	{
		double currentTimeSec = 0.0;
		ksh::Pulse currentPulse = 0;
		double currentBPM = 120.0;

		const ksh::ChartData* pChartData = nullptr;
		
		std::array<LaneState, ksh::kNumBTLanes> btLaneState;
		std::array<LaneState, ksh::kNumFXLanes> fxLaneState;

		CamValues camValues;
	};
}
