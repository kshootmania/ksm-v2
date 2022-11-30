#include "laser_slam_se.hpp"
#include "audio_defines.hpp"

namespace MusicGame::Audio
{
	namespace
	{
		uint32 GetMaxPolyphony(const kson::ChartData& chartData)
		{
			const bool isLegacy = chartData.compat.isKSHVersionOlderThan(kLaserSlamSEMaxPolyphonyLegacyUntilKSHVersion);
			return isLegacy ? kLaserSlamSEMaxPolyphonyLegacy : kLaserSlamSEMaxPolyphony;
		}
	}

	LaserSlamSE::LaserSlamSE(const kson::ChartData& chartData, const kson::TimingCache& timingCache)
		: m_sample("se/chokkaku.wav", GetMaxPolyphony(chartData)) // TODO: 直角音の種類
	{
	}

	void LaserSlamSE::update(const GameStatus& gameStatus)
	{
		for (std::size_t i = 0U; i < kson::kNumLaserLanesSZ; ++i)
		{
			const auto& laneStatus = gameStatus.laserLaneStatus[i];

			if (m_lastPlayedTimeSecs[i] >= laneStatus.lastLaserSlamJudgedTimeSec)
			{
				// 最後に判定した直角LASERの効果音が既に再生済みの場合は何もしない
				continue;
			}

			if (laneStatus.lastLaserSlamJudgedTimeSec > gameStatus.currentTimeSec + kSELatencySec)
			{
				// 最後に判定した直角LASERのタイミングがまだ先にある場合は何もしない
				continue;
			}

			static_assert(kson::kNumLaserLanesSZ == 2U);
			const auto& oppositeLaneStatus = gameStatus.laserLaneStatus[1U - i];
			if (laneStatus.lastJudgedLaserSlamPulse == oppositeLaneStatus.lastJudgedLaserSlamPulse && m_lastPlayedTimeSecs[1U - i] >= oppositeLaneStatus.lastLaserSlamJudgedTimeSec)
			{
				// 2レーン同時直角の場合、他方のレーンで既に再生済みの場合は再生せず、既に再生済みの扱いにする
				// (主にmaxPolyphonyが1でない旧譜面向けの二重再生対策を目的としている。他方のレーンでそれより後ろの直角を判定済みの場合には条件に漏れるが、実用上大きな問題はない)
				m_lastPlayedTimeSecs[i] = laneStatus.lastLaserSlamJudgedTimeSec;
				continue;
			}

			// 直角音を再生
			// TODO: 直角音の音量
			m_sample.play();
			m_lastPlayedTimeSecs[i] = laneStatus.lastLaserSlamJudgedTimeSec;
		}
	}
}
