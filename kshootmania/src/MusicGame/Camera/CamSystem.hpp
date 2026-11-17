#pragma once
#include "kson/ChartData.hpp"

namespace MusicGame::Camera
{
	struct CamStatus
	{
		double zoomBottom = 0.0;
		double shiftX = 0.0;
		double zoomTop = 0.0;
		bool useLegacyZoomTop = false;
		bool useLegacyJdgScale = false;
		double rotationDeg = 0.0;
		double rotationDegHighway = 0.0;
		double rotationDegJdgline = 0.0;
		double rotationDegLayer = 0.0;
		double centerSplit = 0.0;
		double highwayWipe = 0.0;
	};

	class CamSystem
	{
	private:
		CamStatus m_status;

		// KSHバージョン167未満での視点変更の緩和時間適用
		bool m_useLegacyZoomRelaxation = false;
		double m_legacyCurrentZoomTop = 0.0;
		double m_legacyCurrentZoomBottom = 0.0;
		double m_legacyCurrentZoomSide = 0.0;

	public:
		explicit CamSystem(const kson::ChartData& chartData);

		void update(const kson::ChartData& chartData, kson::Pulse currentPulse);

		const CamStatus& status() const
		{
			return m_status;
		}
	};
}
