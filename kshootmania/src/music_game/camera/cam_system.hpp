#pragma once
#include "kson/chart_data.hpp"
#include "cam_pattern/cam_pattern_spin.hpp"

namespace MusicGame::Camera
{
	struct CamStatus
	{
		double zoom = 0.0;
		double shiftX = 0.0;
		double rotationX = 0.0;
		double rotationZ = 0.0;
		double rotationZLane = 0.0;
		double rotationZJdgLine = 0.0;
		double centerSplit = 0.0;
	};

	struct CamPatternStatus
	{
	private:
		Optional<CamPatternStatusSpin> m_spin;

	public:
		void invoke(kson::Pulse currentPulse, const kson::CamPatternInvokeSpin& invokeSpin);

		void apply(kson::Pulse currentPulse, CamStatus& camStatusRef);
	};

	class CamSystem
	{
	private:
		CamStatus m_status;

		CamPatternStatus m_patternStatus;

	public:
		CamSystem() = default;

		void update(const kson::ChartData& chartData, kson::Pulse currentPulse);

		void invokePattern(kson::Pulse currentPulse, const kson::CamPatternInvokeSpin& invoke);

		const CamStatus& status() const
		{
			return m_status;
		}
	};
}
