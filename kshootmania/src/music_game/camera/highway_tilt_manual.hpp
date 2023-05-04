#pragma once

namespace MusicGame::Camera
{
	class HighwayTiltManual
	{
	private:
		double m_radians = 0.0;
		double m_lerpRate = 0.0;

	public:
		HighwayTiltManual() = default;

		void update(const kson::ByPulse<kson::GraphSection>& manual, kson::Pulse currentPulse);

		double radians() const;

		double lerpRate() const;
	};
}
