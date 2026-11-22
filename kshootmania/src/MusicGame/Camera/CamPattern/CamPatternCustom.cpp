#include "CamPatternCustom.hpp"
#include "MusicGame/Camera/CameraMath.hpp"
#include "kson/Util/GraphUtils.hpp"

namespace MusicGame::Camera
{
	namespace
	{
		constexpr double kValueAbsMax = 65535.0;

		// 同じ位置でも向きが異なる直角LASERは別の直角LASERとして扱う
		const kson::CamPatternInvokeCustom* FindCustomEvent(const kson::ByPulseMulti<kson::CamPatternInvokeCustom>& customEvents, kson::Pulse laserSlamPulse, int32 direction)
		{
			const auto [first, last] = customEvents.equal_range(laserSlamPulse);
			const auto itr = std::find_if(first, last, [direction](const auto& kvp) { return kvp.second.d == direction; });
			return (itr != last) ? &itr->second : nullptr;
		}

		void AddCamPatternValues(CamStatus& camStatusRef, const kson::CamGraphs& body, double ry, double scale, int32 flipSign)
		{
			const auto fnValueAt = [ry, scale](const kson::Graph& graph)
			{
				return Clamp(kson::GraphValueAtDouble(graph, ry) * scale, -kValueAbsMax, kValueAbsMax);
			};

			camStatusRef.zoomTop += fnValueAt(body.zoomTop);
			camStatusRef.zoomBottom += fnValueAt(body.zoomBottom);

			camStatusRef.shiftX += ZoomSideToShiftX(fnValueAt(body.zoomSide)) * flipSign;
			camStatusRef.rotationDeg += fnValueAt(body.rotationDeg) * flipSign;
			camStatusRef.rotationDegHighway += fnValueAt(body.rotationDegHighway) * flipSign;
			camStatusRef.rotationDegJdgline += fnValueAt(body.rotationDegJdgline) * flipSign;
			camStatusRef.rotationDegLayer += fnValueAt(body.rotationDegLayer) * flipSign;

			camStatusRef.centerSplit += fnValueAt(body.centerSplit);
			camStatusRef.highwayWipe += fnValueAt(body.highwayWipe);
		}
	}

	CamPatternCustom::CamPatternCustom(const kson::ChartData& chartData, bool invertCamPattern)
		: m_defs(chartData.camera.cam.pattern.def)
		, m_customEvents(chartData.camera.cam.pattern.laser.slamEvent.custom)
		, m_baseFlipSign(invertCamPattern ? -1 : 1)
		, m_slamInvocations(m_defs.size())
	{
		for (std::size_t defIdx = 0; defIdx < m_defs.size(); ++defIdx)
		{
			m_defLengths.push_back(m_defs[defIdx].v.length());

			const auto eventsItr = chartData.camera.cam.pattern.pulseEvent.find(m_defs[defIdx].name);
			if (eventsItr == chartData.camera.cam.pattern.pulseEvent.end())
			{
				continue;
			}
			for (const auto& [y, pulseEvent] : eventsItr->second)
			{
				if (pulseEvent.length <= 0)
				{
					continue;
				}
				m_pulseEvents.push_back(PulseEvent{ .startPulse = y, .durationRelPulse = pulseEvent.length, .scale = pulseEvent.v.scale, .defIdx = defIdx });
				m_pulseEventMaxDurationRelPulse = Max(m_pulseEventMaxDurationRelPulse, pulseEvent.length);
			}
		}
		m_pulseEvents.stable_sort_by([](const PulseEvent& a, const PulseEvent& b) { return a.startPulse < b.startPulse; });
	}

	Optional<std::size_t> CamPatternCustom::findDefIdxForSlam(kson::Pulse laserSlamPulse, int32 direction) const
	{
		for (std::size_t defIdx = 0; defIdx < m_defs.size(); ++defIdx)
		{
			const auto eventsItr = m_customEvents.find(m_defs[defIdx].name);
			if (eventsItr == m_customEvents.end())
			{
				continue;
			}
			if (FindCustomEvent(eventsItr->second, laserSlamPulse, direction) != nullptr)
			{
				return defIdx;
			}
		}
		return none;
	}

	void CamPatternCustom::onLaserSlamJudged(kson::Pulse laserSlamPulse, int32 direction, kson::Pulse currentPulse)
	{
		assert(direction == 1 || direction == -1);

		if (m_alreadyInvokedSlams.contains({ laserSlamPulse, direction }))
		{
			return;
		}

		const Optional<std::size_t> defIdx = findDefIdxForSlam(laserSlamPulse, direction);
		if (!defIdx.has_value())
		{
			return;
		}

		const kson::CamPatternDefKVP& def = m_defs[*defIdx];
		const kson::CamPatternInvokeCustom& customEvent = *FindCustomEvent(m_customEvents.at(def.name), laserSlamPulse, direction);
		m_slamInvocations[*defIdx] = SlamInvocation{
			.startPulse = currentPulse,
			.durationRelPulse = customEvent.length,
			.flipSign = def.v.flipByDirection ? direction : m_baseFlipSign,
			.scale = customEvent.v.scale,
		};
		m_alreadyInvokedSlams.insert({ laserSlamPulse, direction });
	}

	void CamPatternCustom::applyToCamStatus(CamStatus& camStatusRef, kson::Pulse currentPulse) const
	{
		for (std::size_t defIdx = 0; defIdx < m_defs.size(); ++defIdx)
		{
			const SlamInvocation& invocation = m_slamInvocations[defIdx];
			if (currentPulse < invocation.startPulse)
			{
				continue;
			}
			const kson::RelPulse elapsedPulse = currentPulse - invocation.startPulse;
			if (elapsedPulse >= invocation.durationRelPulse)
			{
				continue;
			}

			// 定義の長さを呼び出し時の長さに合わせて伸縮
			const double ry = static_cast<double>(elapsedPulse) / static_cast<double>(invocation.durationRelPulse) * static_cast<double>(m_defLengths[defIdx]);
			AddCamPatternValues(camStatusRef, m_defs[defIdx].v.body, ry, invocation.scale, invocation.flipSign);
		}

		if (currentPulse < 0)
		{
			return;
		}

		// pulse_eventは発動中のものを全て加算
		auto itr = std::upper_bound(m_pulseEvents.begin(), m_pulseEvents.end(), currentPulse - m_pulseEventMaxDurationRelPulse,
			[](kson::Pulse pulse, const PulseEvent& pulseEvent) { return pulse < pulseEvent.startPulse; });
		for (; itr != m_pulseEvents.end() && itr->startPulse <= currentPulse; ++itr)
		{
			const kson::RelPulse elapsedPulse = currentPulse - itr->startPulse;
			if (elapsedPulse >= itr->durationRelPulse)
			{
				continue;
			}
			const double ry = static_cast<double>(elapsedPulse) / static_cast<double>(itr->durationRelPulse) * static_cast<double>(m_defLengths[itr->defIdx]);
			AddCamPatternValues(camStatusRef, m_defs[itr->defIdx].v.body, ry, itr->scale, m_baseFlipSign);
		}
	}

	bool CamPatternCustom::hasEventWithoutSlamShake(kson::Pulse laserSlamPulse, int32 direction) const
	{
		const Optional<std::size_t> defIdx = findDefIdxForSlam(laserSlamPulse, direction);
		return defIdx.has_value() && !m_defs[*defIdx].v.slamShake;
	}
}
