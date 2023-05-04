#include "highway_tilt_manual.hpp"
#include "music_game/game_defines.hpp"
#include "kson/util/graph_utils.hpp"

namespace MusicGame::Camera
{
	void HighwayTiltManual::update(const kson::ByPulse<kson::GraphSection>& manual, kson::Pulse currentPulse)
	{
		const auto value = kson::GraphSectionValueAt(manual, currentPulse);
		Print << value.value_or(0.0);
		m_lerpRate = Clamp(m_lerpRate + Scene::DeltaTime() / 0.5 * (value.has_value() ? 1 : -1), 0.0, 1.0);
		if (value.has_value())
		{
			m_radians = kTiltRadians * value.value();
		}
	}

	double HighwayTiltManual::radians() const
	{
		return m_radians;
	}

	double HighwayTiltManual::lerpRate() const
	{
		return m_lerpRate;
	}
}
