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
			int32 flipSign = 1;
			double scale = 1.0;
		};

		struct PulseEvent
		{
			kson::Pulse startPulse = kson::Pulse{ 0 };
			kson::RelPulse durationRelPulse = kson::RelPulse{ 0 };
			double scale = 1.0;
			std::size_t defIdx = 0;
		};

		const std::vector<kson::CamPatternDefKVP> m_defs;
		const kson::Dict<kson::ByPulseMulti<kson::CamPatternInvokeCustom>> m_customEvents;

		// 方向なしの発動での値の符号(左右反転時は-1)
		const int32 m_baseFlipSign;

		Array<kson::RelPulse> m_defLengths;

		std::set<std::pair<kson::Pulse, int32>> m_alreadyInvokedSlams;

		// m_defsと同じ並び
		Array<SlamInvocation> m_slamInvocations;

		// pulse_eventのカスタム視点パターン(開始位置順)
		Array<PulseEvent> m_pulseEvents;
		kson::RelPulse m_pulseEventMaxDurationRelPulse = kson::RelPulse{ 0 };

		// 直角LASERで発動する視点パターン(複数ある場合は定義順で最初)
		Optional<std::size_t> findDefIdxForSlam(kson::Pulse laserSlamPulse, int32 direction) const;

	public:
		/// @brief コンストラクタ
		/// @param chartData 譜面データ
		/// @param invertCamPattern 方向なしの発動の値を左右反転するか
		CamPatternCustom(const kson::ChartData& chartData, bool invertCamPattern);

		void onLaserSlamJudged(kson::Pulse laserSlamPulse, int32 direction, kson::Pulse currentPulse);

		void applyToCamStatus(CamStatus& camStatusRef, kson::Pulse currentPulse) const;

		bool hasEventWithoutSlamShake(kson::Pulse laserSlamPulse, int32 direction) const;
	};
}
