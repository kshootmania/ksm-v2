#include "FXChipSE.hpp"
#include "AudioDefines.hpp"

namespace MusicGame::Audio
{
	namespace
	{
		bool IsBuiltInSoundName(const std::string& filename)
		{
			return filename == "clap" ||
				filename == "clap_impact" ||
				filename == "clap_punchy" ||
				filename == "snare" ||
				filename == "snare_lo";
		}

		String GetFilePath(FilePathView parentPath, const std::string& filename)
		{
			if (IsBuiltInSoundName(filename))
			{
				// ビルトインの効果音
				return U"se/Note/{}.wav"_fmt(Unicode::FromUTF8(filename));
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
			const bool isLegacy = chartData.compat.isKSHVersionOlderThan(kKeySoundMaxPolyphonyLegacyUntilKSHVersion);
			return isLegacy ? kKeySoundMaxPolyphonyLegacy : kKeySoundMaxPolyphony;
		}
	}

	FXChipSE::FXChipSE(const kson::ChartData& chartData, const kson::TimingCache& timingCache, FilePathView parentPath, bool isAutoPlaySE, double folderVolumeScale)
		: m_isAutoPlaySE(isAutoPlaySE)
		, m_folderVolumeScale(folderVolumeScale)
	{
		// SE自動再生モード用にPulseから秒数への変換マップを作成
		if (m_isAutoPlaySE)
		{
			const auto& chipEvent = chartData.audio.keySound.fx.chipEvent;
			for (const auto& [filename, lanes] : chipEvent)
			{
				for (const auto& lane : lanes)
				{
					for (const auto& [pulse, chipData] : lane)
					{
						if (!m_pulseToSec.contains(pulse))
						{
							m_pulseToSec.emplace(pulse, kson::PulseToSec(pulse, chartData.beat, timingCache));
						}
					}
				}
			}
		}

		// キー音ファイルを読み込み
		const auto& chipEvent = chartData.audio.keySound.fx.chipEvent;
		for (const auto& [filename, lanes] : chipEvent)
		{
			if (m_keySounds.contains(filename))
			{
				continue;
			}

			const FilePath filePath = GetFilePath(parentPath, filename);
			if (!FileSystem::Exists(filePath))
			{
				Logger << U"[ksm warning] failed to open key sound '{}' (filePath:'{}')"_fmt(Unicode::FromUTF8(filename), filePath);
				continue;
			}

			m_keySounds.emplace(filename, ksmaudio::Sample{ filePath.narrow(), GetMaxPolyphony(chartData) });
		}
	}

	void FXChipSE::update(const kson::ChartData& chartData, const GameStatus& gameStatus)
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

	void FXChipSE::updateByNoteTime(const kson::ChartData& chartData, const GameStatus& gameStatus)
	{
		const double currentTimeSecForAudioProc = gameStatus.currentTimeSecForAudioProc + kSELatencySec;
		const auto& chipEvent = chartData.audio.keySound.fx.chipEvent;

		for (const auto& [filename, lanes] : chipEvent)
		{
			for (std::size_t laneIdx = 0U; laneIdx < kson::kNumFXLanesSZ; ++laneIdx)
			{
				if (laneIdx >= lanes.size())
				{
					continue;
				}

				// このレーンのチップイベントを走査
				for (const auto& [chipPulse, chipData] : lanes[laneIdx])
				{
					if (chipPulse <= m_autoPlaySELastPulses[laneIdx])
					{
						// 既に再生済み
						continue;
					}

					const double chipTimeSec = m_pulseToSec.at(chipPulse);
					if (chipTimeSec > currentTimeSecForAudioProc)
					{
						// まだ先のチップなのでこのレーンの処理を終了
						break;
					}

					if (!m_keySounds.contains(filename))
					{
						Logger << U"[ksm warning] key sound not found: '{}'"_fmt(Unicode::FromUTF8(filename));
						m_autoPlaySELastPulses[laneIdx] = chipPulse;
						continue;
					}

					// キー音を再生
					const double volume = chipData.vol;
					m_keySounds.at(filename).play(volume * m_folderVolumeScale);
					m_lastPlayedTimeSecs[laneIdx] = chipTimeSec;
					m_autoPlaySELastPulses[laneIdx] = chipPulse;
				}
			}
		}
	}

	void FXChipSE::updateByJudgment(const kson::ChartData& chartData, const GameStatus& gameStatus)
	{
		const auto& chipEvent = chartData.audio.keySound.fx.chipEvent;

		for (std::size_t laneIdx = 0U; laneIdx < kson::kNumFXLanesSZ; ++laneIdx)
		{
			const auto& laneStatus = gameStatus.fxLaneStatus[laneIdx];
			const kson::Pulse chipPulse = laneStatus.lastJudgedChipPulse;
			const double judgmentTimeSec = laneStatus.lastChipJudgedTimeSec;

			if (m_lastPlayedTimeSecs[laneIdx] >= judgmentTimeSec)
			{
				// 最後に判定したチップの効果音が既に再生済みの場合は何もしない
				continue;
			}

			if (judgmentTimeSec > gameStatus.currentTimeSecForAudioProc + kSELatencySec)
			{
				// 最後に判定したチップのタイミングがまだ先にある場合は何もしない
				continue;
			}

			// このチップに対応するキー音を検索
			for (const auto& [filename, lanes] : chipEvent)
			{
				if (laneIdx >= lanes.size())
				{
					continue;
				}

				const auto it = lanes[laneIdx].find(chipPulse);
				if (it == lanes[laneIdx].end())
				{
					continue;
				}

				if (!m_keySounds.contains(filename))
				{
					Logger << U"[ksm warning] key sound not found: '{}'"_fmt(Unicode::FromUTF8(filename));
					continue;
				}

				// キー音を再生
				const double volume = it->second.vol;
				m_keySounds.at(filename).play(volume * m_folderVolumeScale);
				m_lastPlayedTimeSecs[laneIdx] = judgmentTimeSec;
				break;
			}
		}
	}
}
