#pragma once
#include "kson/ChartData.hpp"
#include "MusicGame/Camera/CamSystem.hpp"
#include "CamPatternSpin.hpp"
#include "CamPatternHalfSpin.hpp"
#include "CamPatternSwing.hpp"
#include "CamPatternCustom.hpp"

namespace MusicGame::Camera
{
	class CamPatternMain
	{
	private:
		CamPatternSpin m_spin;
		CamPatternHalfSpin m_halfSpin;
		CamPatternSwing m_swing;
		CamPatternCustom m_custom;

	public:
		explicit CamPatternMain(const kson::ChartData& chartData);

		void onLaserSlamJudged(kson::Pulse laserSlamPulse, int32 direction, kson::Pulse currentPulse);

		void applyToCamStatus(CamStatus& camStatusRef, kson::Pulse currentPulse) const;

		bool hasSwingEvent(kson::Pulse laserSlamPulse, int32 direction) const;

		bool hasCustomEventWithoutSlamShake(kson::Pulse laserSlamPulse, int32 direction) const;
	};
}
