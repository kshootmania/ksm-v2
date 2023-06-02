#include "cam_system.hpp"
#include "kson/util/graph_utils.hpp"

namespace MusicGame::Camera
{
	void CamPatternStatus::invoke(kson::Pulse currentPulse, const kson::CamPatternInvokeSpin& invoke)
	{
		// Note: Optional::emplaceは既に値がある場合も新しい値で上書きする
		m_spin.emplace(currentPulse, invoke);
	}

	void CamPatternStatus::apply(kson::Pulse currentPulse, CamStatus& camStatusRef)
	{
		if (m_spin.has_value())
		{
			m_spin->apply(currentPulse, camStatusRef);
		}
	}

	void CamSystem::update(const kson::ChartData& chartData, kson::Pulse currentPulse)
	{
		const auto& body = chartData.camera.cam.body;
		m_status.zoom = kson::GraphValueAt(body.zoom, currentPulse);
		m_status.shiftX = kson::GraphValueAt(body.shiftX, currentPulse);
		m_status.rotationX = kson::GraphValueAt(body.rotationX, currentPulse);
		m_status.rotationZ = kson::GraphValueAt(body.rotationZ, currentPulse);
		m_status.centerSplit = kson::GraphValueAt(body.centerSplit, currentPulse);

		m_patternStatus.apply(currentPulse, m_status);
	}

	void CamSystem::invokePattern(kson::Pulse currentPulse, const kson::CamPatternInvokeSpin& invokeSpin)
	{
		m_patternStatus.invoke(currentPulse, invokeSpin);
	}
}
