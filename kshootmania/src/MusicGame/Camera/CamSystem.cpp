#include "CamSystem.hpp"
#include "CameraMath.hpp"
#include "kson/Util/GraphUtils.hpp"

namespace MusicGame::Camera
{
	namespace
	{
		constexpr int kZoomRelaxationChangedKSHVersion = 167;
		constexpr double kZoomRelaxationTimeMs = 40.0;

		double ZoomSideToShiftX(double zoomSide)
		{
			return zoomSide / 6.0;
		}
	}

	CamSystem::CamSystem(const kson::ChartData& chartData)
		: m_useLegacyZoomRelaxation(chartData.compat.isKshVersionOlderThan(kZoomRelaxationChangedKSHVersion))
	{
	}

	void CamSystem::update(const kson::ChartData& chartData, kson::Pulse currentPulse)
	{
		const auto& body = chartData.camera.cam.body;

		// centerSplitとrotationDegは緩和時間なし
		m_status.centerSplit = kson::GraphValueAt(body.centerSplit, currentPulse);
		m_status.rotationDeg = kson::GraphValueAt(body.rotationDeg, currentPulse);

		if (m_useLegacyZoomRelaxation)
		{
			// KSHバージョン167未満の場合、緩和時間40msで目標値に近づける
			const double deltaTimeMs = Scene::DeltaTime() * 1000.0;
			const double targetRate = Min(deltaTimeMs / kZoomRelaxationTimeMs, 1.0);

			const double targetZoomTop = kson::GraphValueAt(body.zoomTop, currentPulse);
			m_legacyCurrentZoomTop = m_legacyCurrentZoomTop * (1.0 - targetRate) + targetZoomTop * targetRate;
			m_status.zoomTop = ScaledLegacyZoomTop(m_legacyCurrentZoomTop);
			m_status.useLegacyZoomTop = true;
			m_status.useLegacyJdgScale = true;

			const double targetZoomBottom = kson::GraphValueAt(body.zoomBottom, currentPulse);
			m_legacyCurrentZoomBottom = m_legacyCurrentZoomBottom * (1.0 - targetRate) + targetZoomBottom * targetRate;
			m_status.zoomBottom = m_legacyCurrentZoomBottom;

			const double targetZoomSide = kson::GraphValueAt(body.zoomSide, currentPulse);
			m_legacyCurrentZoomSide = m_legacyCurrentZoomSide * (1.0 - targetRate) + targetZoomSide * targetRate;
			m_status.shiftX = ZoomSideToShiftX(m_legacyCurrentZoomSide);
		}
		else
		{
			// KSHバージョン167以上の場合、緩和時間なしで即時反映
			m_status.zoomTop = kson::GraphValueAt(body.zoomTop, currentPulse);
			m_status.useLegacyZoomTop = false;
			m_status.useLegacyJdgScale = false;
			m_status.zoomBottom = kson::GraphValueAt(body.zoomBottom, currentPulse);
			m_status.shiftX = ZoomSideToShiftX(kson::GraphValueAt(body.zoomSide, currentPulse));
		}
	}
}
