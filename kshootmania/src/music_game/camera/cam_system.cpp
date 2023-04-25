#include "cam_system.hpp"
#include "kson/util/graph_utils.hpp"

namespace MusicGame::Camera
{
	void CamSystem::update(const kson::ChartData& chartData, kson::Pulse currentPulse)
	{
		const auto& body = chartData.camera.cam.body;
		m_status.zoom = kson::GraphValueAt(body.zoom, currentPulse);
		m_status.shiftX = kson::GraphValueAt(body.shiftX, currentPulse);
		m_status.rotationX = kson::GraphValueAt(body.rotationX, currentPulse);
		m_status.rotationZ = kson::GraphValueAt(body.rotationZ, currentPulse);
	}
}
