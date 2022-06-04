#pragma once
#include "music_game/judgment/judgment_defines.hpp"
#include "kson/common/common.hpp"
#include "kson/chart_data.hpp"

namespace MusicGame::Graphics
{
	constexpr int32 kChipAnimMax = 5;

	struct ChipAnimState
	{
		double startTimeSec = kPastTimeSec;
		Judgment::JudgmentResult type = Judgment::JudgmentResult::kUnspecified;
	};
	
	struct LaneState
	{
		double keyBeamTimeSec = kPastTimeSec;
		Judgment::KeyBeamType keyBeamType = Judgment::KeyBeamType::kDefault;

		std::array<ChipAnimState, kChipAnimMax> chipAnimStateRingBuffer;
		int32 chipAnimStateRingBufferCursor = 0;

		Optional<kson::Pulse> currentLongNotePulse = none;
		double currentLongNoteAnimOffsetTimeSec = kPastTimeSec; // Timing of key press or key release
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

	struct GraphicsUpdateInfo
	{
		double currentTimeSec = 0.0;
		kson::Pulse currentPulse = 0;
		double currentBPM = 120.0;

		const kson::ChartData* pChartData = nullptr;
		
		std::array<LaneState, kson::kNumBTLanes> btLaneState;
		std::array<LaneState, kson::kNumFXLanes> fxLaneState;

		CamValues camValues;

		int32 score = 0;
	};
}
