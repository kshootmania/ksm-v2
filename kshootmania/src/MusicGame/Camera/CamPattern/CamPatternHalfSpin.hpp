#pragma once
#include "kson/ChartData.hpp"
#include "MusicGame/GameDefines.hpp"
#include "MusicGame/Camera/CamSystem.hpp"

namespace MusicGame::Camera
{
	class CamPatternHalfSpin
	{
	private:
		const kson::ByPulse<kson::CamPatternInvokeHalfSpin> m_halfSpinEvents;

		std::unordered_set<kson::Pulse> m_alreadyInvokedEventPulses;

		kson::Pulse m_startPulse = kPastPulse;
		kson::RelPulse m_durationRelPulse = kson::RelPulse{ 0 };
		int32 m_direction = 0;
		double m_deg = 60.0;

	public:
		explicit CamPatternHalfSpin(const kson::ChartData& chartData);

		void onLaserSlamJudged(kson::Pulse laserSlamPulse, int32 direction, kson::Pulse currentPulse);

		void applyToCamStatus(CamStatus& camStatusRef, kson::Pulse currentPulse) const;
	};
}
