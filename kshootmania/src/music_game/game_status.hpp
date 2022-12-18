#pragma once
#include "kson/chart_data.hpp"
#include "game_defines.hpp"
#include "judgment/judgment_defines.hpp"
#include "graphics/graphics_defines.hpp"

namespace MusicGame
{
	struct ChipAnimStatus
	{
		double startTimeSec = kPastTimeSec;
		Judgment::JudgmentResult type = Judgment::JudgmentResult::kUnspecified;
	};

	struct ButtonLaneStatus
	{
		double keyBeamTimeSec = kPastTimeSec;
		Judgment::KeyBeamType keyBeamType = Judgment::KeyBeamType::kDefault;

		std::array<ChipAnimStatus, Graphics::kChipAnimMax> chipAnimStatusRingBuffer;
		std::size_t chipAnimStateRingBufferCursor = 0U;

		// 現在ロングノートが押されているかどうか
		// (ロングノーツが判定ラインに到達するまではボタンを押していてもnoneになる。主に音声エフェクト用)
		Optional<bool> longNotePressed = none;

		// 現在押しているロングノートの始点のPulse値
		// (ロングノーツが判定ラインに到達するまでにボタンを押した場合でもnoneではなくPulse値になる。主に判定グラフィックス用)
		Optional<kson::Pulse> currentLongNotePulse = none;

		// 直近でキーを押した、または離した時間(秒)
		double currentLongNoteAnimOffsetTimeSec = kPastTimeSec;
	};

	struct LaserLaneStatus
	{
		// プレイヤーのカーソル横位置(0.0～1.0)
		Optional<double> cursorX = none;

		// ノーツ側の理想カーソル横位置(0.0～1.0)
		Optional<double> noteCursorX = none;

		// ノーツ側の表示用理想カーソル横位置(0.0～1.0)
		// 
		// LASERの判定タイミングをずらした際にカーソルが表示ノーツからずれないように別途設けている。
		// cursorXがnoteCursorXから一定以内の距離であればnoteVisualCursorXをカーソル表示位置として使用する。
		Optional<double> noteVisualCursorX = none;

		// 現在判定しているLASERセクションの始点のPulse値
		// (クリティカル判定でない場合にもnoneではなくPulse値になる。主に判定グラフィックス用)
		Optional<kson::Pulse> currentLaserSectionPulse = none;

		// 最後に直角LASERを判定した時間
		// (先行判定の場合は直角LASERの時間、後で判定した場合は判定した時間が入る)
		double lastLaserSlamJudgedTimeSec = kPastTimeSec;

		// 最後に判定した直角LASERのPulse値
		kson::Pulse lastJudgedLaserSlamPulse = kPastPulse;

		bool isCursorInCriticalJudgmentRange() const
		{
			return cursorX.has_value() && noteCursorX.has_value() && Judgment::IsLaserCursorInCriticalJudgmentRange(cursorX.value(), noteCursorX.value());
		}
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
		
		std::array<ButtonLaneStatus, kson::kNumBTLanesSZ> btLaneStatus;
		std::array<ButtonLaneStatus, kson::kNumFXLanesSZ> fxLaneStatus;
		std::array<LaserLaneStatus, kson::kNumLaserLanesSZ> laserLaneStatus;

		std::size_t lastPressedLongFXNoteLaneIdx = 0U; // For audio effect parameter priority

		CamStatus camStatus;

		int32 score = 0;
	};
}
