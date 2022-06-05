#pragma once
#include "kson/chart_data.hpp"
#include "judgment/judgment_defines.hpp"
#include "graphics/graphics_defines.hpp"

namespace MusicGame
{
	struct ChipAnimStatus
	{
		double startTimeSec = kPastTimeSec;
		Judgment::JudgmentResult type = Judgment::JudgmentResult::kUnspecified;
	};

	struct LaneStatus
	{
		double keyBeamTimeSec = kPastTimeSec;
		Judgment::KeyBeamType keyBeamType = Judgment::KeyBeamType::kDefault;

		std::array<ChipAnimStatus, Graphics::kChipAnimMax> chipAnimStatusRingBuffer;
		std::size_t chipAnimStateRingBufferCursor = 0U;

		// This value will be none while a long note pressed before the note start
		// (for audio effects)
		Optional<bool> longNotePressed = none;

		// This value will NOT be none while a long note is pressed before the note start
		// (for graphics)
		Optional<kson::Pulse> currentLongNotePulse = none;

		// Timing of key press or key release
		double currentLongNoteAnimOffsetTimeSec = kPastTimeSec;
	};

	struct CamStatus
	{
		double zoom = 0.0;
		double shiftX = 0.0;
		double rotationX = 0.0;
		double rotationZ = 0.0;
		double rotationZLane = 0.0;
		double rotationZJdgLine = 0.0;
	};

	struct GameStatus
	{
		double currentTimeSec = 0.0;
		kson::Pulse currentPulse = 0;
		double currentBPM = 120.0;
		
		std::array<LaneStatus, kson::kNumBTLanes> btLaneStatus;
		std::array<LaneStatus, kson::kNumFXLanes> fxLaneStatus;

		std::size_t lastPressedLongFXNoteLaneIdx = 0U; // For audio effect parameter priority

		CamStatus camStatus;

		int32 score = 0;
	};
}
