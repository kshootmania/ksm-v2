#pragma once
#include "kson/ChartData.hpp"
#include "MusicGame/GameDefines.hpp"
#include "MusicGame/Camera/CamSystem.hpp"

namespace MusicGame::Camera
{
	class CamPatternCustom
	{
	private:
		// 直角LASERによる発動(定義毎に同時に1つ)
		struct SlamInvocation
		{
			kson::Pulse startPulse = kPastPulse;
			kson::RelPulse durationRelPulse = kson::RelPulse{ 0 };
			int32 direction = 0;
			double scale = 1.0;
		};

		const std::vector<kson::CamPatternDefKVP> m_defs;
		const kson::Dict<kson::ByPulseMulti<kson::CamPatternInvokeCustom>> m_customEvents;

		Array<kson::RelPulse> m_defLengths;

		std::set<std::pair<kson::Pulse, int32>> m_alreadyInvokedSlams;

		// m_defsと同じ並び
		Array<SlamInvocation> m_slamInvocations;

		// 直角LASERで発動する視点パターン(複数ある場合は定義順で最初)
		Optional<std::size_t> findDefIdxForSlam(kson::Pulse laserSlamPulse, int32 direction) const;

	public:
		explicit CamPatternCustom(const kson::ChartData& chartData);

		void onLaserSlamJudged(kson::Pulse laserSlamPulse, int32 direction, kson::Pulse currentPulse);

		void applyToCamStatus(CamStatus& camStatusRef, kson::Pulse currentPulse) const;

		bool hasEvent(kson::Pulse laserSlamPulse, int32 direction) const;
	};
}
