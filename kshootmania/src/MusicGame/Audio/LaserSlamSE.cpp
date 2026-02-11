#include "LaserSlamSE.hpp"
#include "AudioDefines.hpp"

namespace MusicGame::Audio
{
	namespace
	{
		constexpr const char* kDefaultSlamSoundPath = "se/chokkaku.wav";

		bool IsBuiltInSlamSoundName(const std::string& filename)
		{
			return filename == "up" ||
				filename == "down" ||
				filename == "swing" ||
				filename == "mute";
		}

		String GetFilePath(FilePathView parentPath, const std::string& filename)
		{
			if (IsBuiltInSlamSoundName(filename))
			{
				// ビルトインの直角音
				return U"se/chokkaku_{}.wav"_fmt(Unicode::FromUTF8(filename));
			}
			else
			{
				// 譜面と同じフォルダの音声ファイル
				return FileSystem::PathAppend(parentPath, Unicode::FromUTF8(filename));
			}
		}

		DWORD GetMaxPolyphony(const kson::ChartData& chartData)
		{
			// 旧バージョンの譜面では最大同時再生数が異なる
			const bool isLegacy = chartData.compat.isKshVersionOlderThan(kLaserSlamSEMaxPolyphonyLegacyUntilKSHVersion);
			return isLegacy ? kLaserSlamSEMaxPolyphonyLegacy : kLaserSlamSEMaxPolyphony;
		}
	}

	LaserSlamSE::LaserSlamSE(const kson::ChartData& chartData, const kson::TimingCache& timingCache, FilePathView parentPath, bool isAutoPlaySE, double folderVolumeScale)
		: m_defaultSlamSound(kDefaultSlamSoundPath, GetMaxPolyphony(chartData))
		, m_isAutoPlaySE(isAutoPlaySE)
		, m_folderVolumeScale(folderVolumeScale)
	{
		// SE自動再生モード用にPulseから秒数への変換マップを作成
		if (m_isAutoPlaySE)
		{
			for (std::size_t laneIdx = 0U; laneIdx < kson::kNumLaserLanesSZ; ++laneIdx)
			{
				const auto& lane = chartData.note.laser.at(laneIdx);
				for (const auto& [graphSectionY, graphSection] : lane)
				{
					for (const auto& [slamRy, graphPoint] : graphSection.v)
					{
						if (graphPoint.v.vf == graphPoint.v.v)
						{
							// 直角ではない
							continue;
						}

						const kson::Pulse slamY = graphSectionY + slamRy;
						if (!m_pulseToSec.contains(slamY))
						{
							m_pulseToSec.emplace(slamY, kson::PulseToSec(slamY, chartData.beat, timingCache));
						}
					}
				}
			}
		}

		// slamEventで指定された直角音ファイルを読み込み
		const auto& slamEvent = chartData.audio.keySound.laser.slamEvent;
		for (const auto& [filename, pulseSet] : slamEvent)
		{
			if (m_slamSounds.contains(filename))
			{
				continue;
			}

			const FilePath filePath = GetFilePath(parentPath, filename);
			if (!FileSystem::Exists(filePath))
			{
				Logger << U"[ksm warning] failed to open slam sound '{}' (filePath:'{}')"_fmt(Unicode::FromUTF8(filename), filePath);
				continue;
			}

			m_slamSounds.emplace(filename, ksmaudio::Sample{ filePath.narrow(), GetMaxPolyphony(chartData) });
		}
	}

	void LaserSlamSE::update(const kson::ChartData& chartData, const GameStatus& gameStatus)
	{
		if (m_isAutoPlaySE)
		{
			// SE自動再生モードの場合、ノーツタイミングで効果音を再生
			updateByNoteTime(chartData, gameStatus);
		}
		else
		{
			// 判定したタイミングで効果音を再生
			updateByJudgment(chartData, gameStatus);
		}
	}

	void LaserSlamSE::updateByNoteTime(const kson::ChartData& chartData, const GameStatus& gameStatus)
	{
		const double currentTimeSecForAudioProc = gameStatus.currentTimeSecForAudioProc + kSELatencySec;

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
			const auto& graphPoint = graphSection.v.at(slamRy);
			const double width = AbsDiff(graphPoint.v.v, graphPoint.v.vf) * graphSection.w;
			return Clamp(width * 2, 0.0, 1.0);
		};

		// レーン毎に直角LASERを走査
		for (std::size_t i = 0U; i < kson::kNumLaserLanesSZ; ++i)
		{
			const auto& lane = chartData.note.laser.at(i);

			for (const auto& [graphSectionY, graphSection] : lane)
			{
				for (const auto& [slamRy, graphPoint] : graphSection.v)
				{
					if (graphPoint.v.vf == graphPoint.v.v)
					{
						// 直角ではない
						continue;
					}

					const kson::Pulse slamY = graphSectionY + slamRy;

					if (slamY <= m_autoPlaySELastPulses[i])
					{
						// 既に再生済み
						continue;
					}

					if (!m_pulseToSec.contains(slamY))
					{
						Logger << U"[ksm warning] slam pulse not found in pulseToSec map: {}"_fmt(slamY);
						m_autoPlaySELastPulses[i] = slamY;
						continue;
					}

					const double slamTimeSec = m_pulseToSec.at(slamY);
					if (slamTimeSec > currentTimeSecForAudioProc)
					{
						// まだ先の直角なのでこのレーンの処理を終了
						break;
					}

					// 直角音の音量を取得
					const auto& volByPulse = chartData.audio.keySound.laser.vol;
					const double volume = volByPulse.empty() ? kLaserSlamDefaultVolume : kson::ValueItrAt(volByPulse, slamY)->second;

					const double volumeScaleByNote = fnGetVolumeScaleByNote(chartData, i, slamY);

					static_assert(kson::kNumLaserLanesSZ == 2U);
					const std::size_t oppositeLaneIdx = 1U - i;

					// 2レーン同時直角の場合、大きい方の音量を優先する
					if (slamY == m_autoPlaySELastPulses[oppositeLaneIdx])
					{
						const double oppositeVolumeScaleByNote = fnGetVolumeScaleByNote(chartData, oppositeLaneIdx, slamY);
						if (oppositeVolumeScaleByNote > volumeScaleByNote ||
							(oppositeVolumeScaleByNote == volumeScaleByNote && m_lastPlayedTimeSecs[oppositeLaneIdx] >= slamTimeSec))
						{
							// 他方のレーンの方が音量が大きい場合は再生せず(もし完全に同じ音量の場合は他方のレーンで既に再生済みの場合は再生せず)、既に再生済みの扱いにする
							m_lastPlayedTimeSecs[i] = slamTimeSec;
							m_autoPlaySELastPulses[i] = slamY;
							continue;
						}
					}

					// このパルス位置に対応する直角音の種類を検索
					const auto& slamEvent = chartData.audio.keySound.laser.slamEvent;
					ksmaudio::Sample* pSlamSound = &m_defaultSlamSound;
					for (const auto& [filename, pulseSet] : slamEvent)
					{
						if (pulseSet.contains(slamY))
						{
							if (m_slamSounds.contains(filename))
							{
								pSlamSound = &m_slamSounds.at(filename);
							}
							else
							{
								Logger << U"[ksm warning] slam sound not found: '{}'"_fmt(Unicode::FromUTF8(filename));
							}
							break;
						}
					}

					// 直角音を再生
					pSlamSound->play(volume * volumeScaleByNote * m_folderVolumeScale);
					m_lastPlayedTimeSecs[i] = slamTimeSec;
					m_autoPlaySELastPulses[i] = slamY;
				}
			}
		}
	}

	void LaserSlamSE::updateByJudgment(const kson::ChartData& chartData, const GameStatus& gameStatus)
	{
		for (std::size_t i = 0U; i < kson::kNumLaserLanesSZ; ++i)
		{
			const auto& laneStatus = gameStatus.laserLaneStatus[i];

			if (m_lastPlayedTimeSecs[i] >= laneStatus.lastLaserSlamJudgedTimeSec)
			{
				// 最後に判定した直角LASERの効果音が既に再生済みの場合は何もしない
				continue;
			}

			if (laneStatus.lastLaserSlamJudgedTimeSec > gameStatus.currentTimeSecForAudioProc + kSELatencySec)
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
				const auto& graphPoint = graphSection.v.at(slamRy);
				const double width = AbsDiff(graphPoint.v.v, graphPoint.v.vf) * graphSection.w;
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

			// このパルス位置に対応する直角音の種類を検索
			const auto& slamEvent = chartData.audio.keySound.laser.slamEvent;
			ksmaudio::Sample* pSlamSound = &m_defaultSlamSound;
			for (const auto& [filename, pulseSet] : slamEvent)
			{
				if (pulseSet.contains(slamY))
				{
					if (m_slamSounds.contains(filename))
					{
						pSlamSound = &m_slamSounds.at(filename);
					}
					else
					{
						Logger << U"[ksm warning] slam sound not found: '{}'"_fmt(Unicode::FromUTF8(filename));
					}
					break;
				}
			}

			// 直角音を再生
			pSlamSound->play(volume * volumeScaleByNote * m_folderVolumeScale);
			m_lastPlayedTimeSecs[i] = laneStatus.lastLaserSlamJudgedTimeSec;
		}
	}
}
