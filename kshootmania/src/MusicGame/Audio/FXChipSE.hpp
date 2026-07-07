#pragma once
#include "MusicGame/GameDefines.hpp"
#include "MusicGame/GameStatus.hpp"
#include "kson/ChartData.hpp"
#include "kson/Util/TimingUtils.hpp"
#include "ksmaudio/ksmaudio.hpp"

namespace MusicGame::Audio
{
	/// @brief チップFXノーツの判定情報
	struct FXChipJudgedStatus
	{
		kson::Pulse lastJudgedChipPulse = kPastPulse;

		double lastChipJudgedTimeSec = kPastTimeSec;
	};

	class FXChipSE
	{
	private:
		std::unordered_map<std::string, ksmaudio::Sample> m_keySounds;

		std::array<double, kson::kNumFXLanesSZ> m_lastPlayedTimeSecs = { kPastTimeSec, kPastTimeSec };

		bool m_isAutoPlaySE;

		double m_folderVolumeScale;

		std::array<kson::Pulse, kson::kNumFXLanesSZ> m_autoPlaySELastPulses = { 0, 0 };

		std::map<kson::Pulse, double> m_pulseToSec;

		void updateByNoteTime(const kson::ChartData& chartData, const GameStatus& gameStatus);

		void updateByJudgment(const kson::ChartData& chartData, const GameStatus& gameStatus, const std::array<FXChipJudgedStatus, kson::kNumFXLanesSZ>& judgedStatuses);

	public:
		explicit FXChipSE(const kson::ChartData& chartData, const kson::TimingCache& timingCache, FilePathView parentPath, bool isAutoPlaySE, double folderVolumeScale);

		void update(const kson::ChartData& chartData, const GameStatus& gameStatus, const std::array<FXChipJudgedStatus, kson::kNumFXLanesSZ>& judgedStatuses);
	};
}
