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

		void AddCamPatternValues(CamStatus& camStatusRef, const kson::CamGraphs& body, double ry, double scale, int32 direction)
		{
			const auto fnValueAt = [ry, scale](const kson::Graph& graph)
			{
				return Clamp(kson::GraphValueAtDouble(graph, ry) * scale, -kValueAbsMax, kValueAbsMax);
			};

			camStatusRef.zoomTop += fnValueAt(body.zoomTop);
			camStatusRef.zoomBottom += fnValueAt(body.zoomBottom);

			// 左向き直角LASERの場合はzoom_sideとrotation_deg系を左右反転
			camStatusRef.shiftX += ZoomSideToShiftX(fnValueAt(body.zoomSide)) * direction;
			camStatusRef.rotationDeg += fnValueAt(body.rotationDeg) * direction;
			camStatusRef.rotationDegHighway += fnValueAt(body.rotationDegHighway) * direction;
			camStatusRef.rotationDegJdgline += fnValueAt(body.rotationDegJdgline) * direction;
			camStatusRef.rotationDegLayer += fnValueAt(body.rotationDegLayer) * direction;

			camStatusRef.centerSplit += fnValueAt(body.centerSplit);
			camStatusRef.highwayWipe += fnValueAt(body.highwayWipe);
		}
	}

	CamPatternCustom::CamPatternCustom(const kson::ChartData& chartData)
		: m_defs(chartData.camera.cam.pattern.def)
		, m_customEvents(chartData.camera.cam.pattern.laser.slamEvent.custom)
		, m_slamInvocations(m_defs.size())
	{
		for (const auto& [name, def] : m_defs)
		{
			m_defLengths.push_back(def.length());
		}
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
			.direction = direction,
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
			AddCamPatternValues(camStatusRef, m_defs[defIdx].v.body, ry, invocation.scale, invocation.direction);
		}
	}

	bool CamPatternCustom::hasEventWithoutSlamShake(kson::Pulse laserSlamPulse, int32 direction) const
	{
		const Optional<std::size_t> defIdx = findDefIdxForSlam(laserSlamPulse, direction);
		return defIdx.has_value() && !m_defs[*defIdx].v.slamShake;
	}
}
