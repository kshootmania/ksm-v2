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

	LaserSlamSE::LaserSlamSE(const kson::ChartData& chartData)
		: m_sample("se/chokkaku.wav", GetMaxPolyphony(chartData)) // TODO: 直角音の種類
	{
	}

	void LaserSlamSE::update(const kson::ChartData& chartData, const GameStatus& gameStatus)
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
			const std::size_t oppositeLaneIdx = 1U - i;
			const kson::Pulse slamY = laneStatus.lastJudgedLaserSlamPulse;

			// 直角音の音量を取得
			const auto& volByPulse = chartData.audio.keySound.laser.vol;
			const double volume = volByPulse.empty() ? kLaserSlamDefaultVolume : kson::ValueItrAt(volByPulse, slamY)->second;

			const auto fnGetVolumeScaleByNote = [](const kson::ChartData& chartData, std::size_t laneIdx, kson::Pulse slamY) -> double
			{
				if (!chartData.audio.keySound.laser.legacy.volAuto)
				{
					return 1.0;
				}

				// 旧バージョンの直角音の音量を直角幅に応じて自動的に決めるオプション(chokkakuautovol)が有効の場合は、直角の横幅をもとに音量スケールを決定
				const auto& lane = chartData.note.laser.at(laneIdx);
				const auto graphSectionItr = kson::GraphSectionAt(lane, slamY);
				if (graphSectionItr == lane.end())
				{
					assert(false && "graph section must exist here");
					return 1.0;
				}

				const auto& [graphSectionY, graphSection] = *graphSectionItr;
				const kson::RelPulse slamRy = slamY - graphSectionY;
				if (!graphSection.v.contains(slamRy))
				{
					assert(false && "graph value must exist here");
					return 1.0;
				}
				const auto& graphValue = graphSection.v.at(slamRy);
				const double width = AbsDiff(graphValue.v, graphValue.vf) * graphSection.w;
				return Clamp(width * 2, 0.0, 1.0);
			};

			const double volumeScaleByNote = fnGetVolumeScaleByNote(chartData, i, slamY);
			const auto& oppositeLaneStatus = gameStatus.laserLaneStatus[oppositeLaneIdx];

			// 2レーン同時直角の場合、大きい方の音量を優先する
			if (slamY == oppositeLaneStatus.lastJudgedLaserSlamPulse)
			{
				const double oppositeVolumeScaleByNote = fnGetVolumeScaleByNote(chartData, oppositeLaneIdx, slamY);
				if (oppositeVolumeScaleByNote > volumeScaleByNote ||
					(oppositeVolumeScaleByNote == volumeScaleByNote && m_lastPlayedTimeSecs[oppositeLaneIdx] >= oppositeLaneStatus.lastLaserSlamJudgedTimeSec /* 最終再生時間が判定時間以降ということは再生済み */))
				{
					// 他方のレーンの方が音量が大きい場合は再生せず(もし完全に同じ音量の場合は他方のレーンで既に再生済みの場合は再生せず)、既に再生済みの扱いにする
					m_lastPlayedTimeSecs[i] = laneStatus.lastLaserSlamJudgedTimeSec;
					continue;
				}
			}

			// 直角音を再生
			m_sample.play(volume * volumeScaleByNote);
			m_lastPlayedTimeSecs[i] = laneStatus.lastLaserSlamJudgedTimeSec;
		}
	}
}
