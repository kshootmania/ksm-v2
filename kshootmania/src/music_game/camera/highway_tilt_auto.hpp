#pragma once

namespace MusicGame::Camera
{
	class HighwayTiltAuto
	{
	private:
		double m_smoothedTiltFactor = 0.0;

	public:
		HighwayTiltAuto() = default;

		void update(double tiltFactor);

		double radians() const;
	};
}
