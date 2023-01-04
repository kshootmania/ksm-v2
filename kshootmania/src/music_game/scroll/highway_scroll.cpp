#include "highway_scroll.hpp"

namespace MusicGame::Scroll
{
	namespace
	{
		// ハイスピードの基準ピクセル数
		// HSP版:
		//     https://github.com/m4saka/kshootmania-v1-hsp/blob/08275836547c7792a6d4f59037e56e947f2979c3/src/scene/play/play_key_input.hsp#L429-L440
		//     https://github.com/m4saka/kshootmania-v1-hsp/blob/d2811a09e2d75dad5cc152d7c4073897061addb7/src/scene/play/play_draw_frame.hsp#L96
		// 上記の「108/2」と「10」を乗算した値にあたる
		constexpr double kBasePixels = 540.0;

		kson::Pulse LastNoteEndPulseButtonLane(const kson::ByPulse<kson::Interval>& lane)
		{
			if (lane.empty())
			{
				return kson::Pulse{ 0 };
			}

			const auto& [y, lastNote] = *lane.rbegin();

			return y + lastNote.length;
		}

		kson::Pulse LastNoteEndPulseLaserLane(const kson::ByPulse<kson::LaserSection>& lane)
		{
			if (lane.empty())
			{
				return kson::Pulse{ 0 };
			}

			const auto& [y, lastSection] = *lane.rbegin();
			if (lastSection.v.empty())
			{
				assert(false && "Laser section must not be empty");
				return y;
			}

			const auto& [ry, lastPoint] = *lastSection.v.rbegin();
			return y + ry;
		}

		kson::Pulse LastNoteEndPulse(const kson::ChartData& chartData)
		{
			auto maxPulse = kson::Pulse{ 0 };
			for (const auto& lane : chartData.note.bt)
			{
				const kson::Pulse y = LastNoteEndPulseButtonLane(lane);
				if (y > maxPulse)
				{
					maxPulse = y;
				}
			}
			for (const auto& lane : chartData.note.fx)
			{
				const kson::Pulse y = LastNoteEndPulseButtonLane(lane);
				if (y > maxPulse)
				{
					maxPulse = y;
				}
			}
			for (const auto& lane : chartData.note.laser)
			{
				const kson::Pulse y = LastNoteEndPulseLaserLane(lane);
				if (y > maxPulse)
				{
					maxPulse = y;
				}
			}
			return maxPulse;
		}

		/// @brief BPMの最頻値(累計Pulse値が最も大きいBPM)を返す
		/// @param chartData 譜面データ
		/// @return BPM
		/// @remarks BPM変更が2個以上ある場合、小数部分は無視する(そのため、BPMが整数でない場合にハイスピード値に若干の誤差が出るが、仕様)
		double GetModeBPM(const kson::ChartData& chartData)
		{
			constexpr double kErrorBPM = 120.0;

			if (chartData.beat.bpm.empty())
			{
				// BPMは1個以上存在するはず
				assert(false && "kson.beat.bpm is empty");
				return kErrorBPM;
			}

			if (chartData.beat.bpm.size() == 1U)
			{
				// BPMが1個の場合はその値を返せばOK
				return chartData.beat.bpm.begin()->second;
			}

			// 各BPM値が占めるPulse値の合計を調べる
			std::unordered_map<int32, kson::RelPulse> bpmTotalPulses;
			kson::Pulse prevY = kson::Pulse{ 0 };
			Optional<int32> prevBPMInt = none;
			for (const auto& [y, bpm] : chartData.beat.bpm)
			{
				if (y < prevY)
				{
					assert(false && "y must be larger than or equal to prevY in kson.beat.bpm");
					return kErrorBPM;
				}
				if (prevBPMInt.has_value())
				{
					// 存在しない場合はoperator[]でゼロ初期化されるので明示的な代入不要
					bpmTotalPulses[prevBPMInt.value()] += y - prevY;
				}

				prevY = y;
				prevBPMInt = static_cast<int32>(bpm);
			}

			// 最終BPM変化を加味する
			// (最終ノーツのPulse値を調べ、最終BPM変化より後ろであれば加味)
			const kson::Pulse lastNoteEndPulse = LastNoteEndPulse(chartData);
			if (lastNoteEndPulse > prevY)
			{
				if (!prevBPMInt.has_value())
				{
					assert(false && "prevBPMInt must not be none after the loop");
					return kErrorBPM;
				}
				bpmTotalPulses[prevBPMInt.value()] += lastNoteEndPulse - prevY;
			}

			if (bpmTotalPulses.empty())
			{
				assert(false && "bpmPulses must not be empty");
				return kErrorBPM;
			}

			// 最も累計Pulse値が大きいBPMを調べる
			const auto itr = std::max_element(
				bpmTotalPulses.begin(),
				bpmTotalPulses.end(),
				[](const auto& a, const auto& b) { return a.second < b.second; });

			if (itr == bpmTotalPulses.end())
			{
				assert(false && "max_element must not return end iterator because bpmPulses must not be empty");
				return kErrorBPM;
			}

			const auto& [modeBPM, modeBPMTotalPulse] = *itr;
			return static_cast<double>(modeBPM);
		}

		/// @brief ハイスピード係数を求める
		/// @param hispeedSetting ハイスピード設定
		/// @param stdBPM 基準BPM
		/// @return ハイスピード係数
		double HispeedFactor(const HispeedSetting& hispeedSetting, double stdBPM)
		{
			switch (hispeedSetting.type)
			{
			case HispeedType::XMod:
				return static_cast<double>(hispeedSetting.value) / 10;

			case HispeedType::OMod:
				return static_cast<double>(hispeedSetting.value) / stdBPM;

			case HispeedType::CMod:
				return static_cast<double>(hispeedSetting.value);

			default:
				assert(false && "Unknown hispeed type");
				return static_cast<double>(hispeedSetting.value);
			}
		}

		/// @brief ハイスピード値を求める
		/// @param hispeedSetting ハイスピード設定
		/// @param currentBPM 現在のBPM
		/// @param stdBPM 基準BPM
		/// @return ハイスピード係数
		/// @note HispeedFactorにBPMをかけた値と基本的に同じだが、C-modで設定値をそのまま返す点、整数のまま計算する点が異なる
		int32 CurrentHispeed(const HispeedSetting& hispeedSetting, double currentBPM, double stdBPM)
		{
			switch (hispeedSetting.type)
			{
			case HispeedType::XMod:
				return static_cast<int32>(currentBPM) * hispeedSetting.value / 10;

			case HispeedType::OMod:
				return static_cast<int32>(currentBPM * hispeedSetting.value / stdBPM);

			case HispeedType::CMod:
				return hispeedSetting.value;

			default:
				assert(false && "Unknown hispeed type");
				return static_cast<int32>(currentBPM);
			}
		}
	}

	HighwayScrollContext::HighwayScrollContext(const HighwayScroll* pHighwayScroll, const kson::BeatInfo* pBeatInfo, const kson::TimingCache* pTimingCache, const GameStatus* pGameStatus)
		: m_pHighwayScroll(pHighwayScroll)
		, m_pBeatInfo(pBeatInfo)
		, m_pTimingCache(pTimingCache)
		, m_pGameStatus(pGameStatus)
	{
	}

	HighwayScrollContext::~HighwayScrollContext() = default;

	int32 HighwayScrollContext::getPositionY(kson::Pulse pulse) const
	{
		return m_pHighwayScroll->getPositionY(pulse, *m_pBeatInfo, *m_pTimingCache, *m_pGameStatus);
	}

	const HighwayScroll& HighwayScrollContext::highwayScroll() const
	{
		return *m_pHighwayScroll;
	}

	double HighwayScroll::pulseToSec(kson::Pulse pulse, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache) const
	{
		// mutableな変数を操作するのでロック
		std::lock_guard lock(m_mutex);

		// 既に結果がキャッシュ上にあればキャッシュの値を返す
		if (m_pulseToSecCache.contains(pulse))
		{
			return m_pulseToSecCache.at(pulse);
		}

		// キャッシュ上になければ計算してキャッシュする
		const double sec = kson::PulseToSec(pulse, beatInfo, timingCache);
		m_pulseToSecCache.emplace(pulse, sec);
		return sec;
	}

	double HighwayScroll::getRelPulseEquvalent(kson::Pulse pulse, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache, const GameStatus& gameStatus) const
	{
		if (m_hispeedSetting.type == HispeedType::CMod)
		{
			const double sec = pulseToSec(pulse, beatInfo, timingCache);
			const double relTimeSec = sec - gameStatus.currentTimeSec;
			return relTimeSec / 60 * kson::kResolution;
		}
		else
		{
			return static_cast<double>(pulse - gameStatus.currentPulse);
		}
	}

	HighwayScroll::HighwayScroll(const kson::ChartData& chartData)
		: m_stdBPM(chartData.meta.stdBPM > 0.0 ? chartData.meta.stdBPM : GetModeBPM(chartData))
	{
	}

	void HighwayScroll::update(const HispeedSetting& hispeedSetting, double currentBPM)
	{
		m_hispeedSetting = hispeedSetting;
		m_hispeedFactor = HispeedFactor(hispeedSetting, m_stdBPM);
		m_currentHispeed = CurrentHispeed(hispeedSetting, currentBPM, m_stdBPM);
	}

	int32 HighwayScroll::getPositionY(kson::Pulse pulse, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache, const GameStatus& gameStatus) const
	{
		assert(m_hispeedFactor != 0.0 && "HighwayScroll::update() must be called at least once before HighwayScroll::getPositionY()");

		const double relPulseEquivalent = getRelPulseEquvalent(pulse, beatInfo, timingCache, gameStatus);
		return Graphics::kHighwayTextureSize.y - static_cast<int32>(relPulseEquivalent * kBasePixels * m_hispeedFactor / kson::kResolution4);
	}

	const HispeedSetting& HighwayScroll::hispeedSetting() const
	{
		return m_hispeedSetting;
	}

	int32 HighwayScroll::currentHispeed() const
	{
		return m_currentHispeed;
	}
}
