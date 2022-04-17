#pragma once

namespace MusicGame::Graphics
{
	class HighwayTilt
	{
	private:
		double m_smoothedTiltFactor = 0.0;

	public:
		HighwayTilt() = default;

		void update(double tiltFactor);

		double radians() const;
	};
}
