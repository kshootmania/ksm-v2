#include "HighwayScroll.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace MusicGame::Scroll
{
	namespace
	{
		// ハイスピードの基準ピクセル数
		// HSP版:
		//     https://github.com/kshootmania/ksm-v1/blob/08275836547c7792a6d4f59037e56e947f2979c3/src/scene/play/play_key_input.hsp#L429-L440
		//     https://github.com/kshootmania/ksm-v1/blob/d2811a09e2d75dad5cc152d7c4073897061addb7/src/scene/play/play_draw_frame.hsp#L96
		// 上記の「108/2」と「10」を乗算した値にあたる
		constexpr double kBasePixels = 540.0;
		constexpr kson::RelPulse kDrawRangeSampleInterval = kson::kResolution4 / 16;

		/// @brief scrollSpeedに負の値が含まれているかを判定
		/// @param beatInfo kson.beat
		/// @return 負の値が含まれていればtrue
		bool HasNegativeScrollSpeed(const kson::BeatInfo& beatInfo)
		{
			// scrollSpeedグラフに負の値が1つでも含まれていればtrueを返す
			for (const auto& [pulse, graphValue] : beatInfo.scrollSpeed)
			{
				if (graphValue.v.v < 0.0 || graphValue.v.vf < 0.0)
				{
					return true;
				}
			}
			return false;
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

		/// @brief scrollSpeedの1区間を台形則で積分(始点After/終点Beforeで即時変更に対応)
		double CalcScrollSpeedSegmentIntegral(const kson::Graph& scrollSpeed, double startPulse, double endPulse)
		{
			const double startSpeed = kson::GraphValueAtDouble(scrollSpeed, startPulse, kson::GraphSide::After);
			const double endSpeed = kson::GraphValueAtDouble(scrollSpeed, endPulse, kson::GraphSide::Before);
			return (endPulse - startPulse) * (startSpeed + endSpeed) / 2.0;
		}
	}

	HighwayScrollContext::HighwayScrollContext(const HighwayScroll* pHighwayScroll, const kson::BeatInfo* pBeatInfo, const kson::TimingCache* pTimingCache, const GameStatus* pGameStatus)
		: m_pHighwayScroll(pHighwayScroll)
		, m_pBeatInfo(pBeatInfo)
		, m_pTimingCache(pTimingCache)
		, m_pGameStatus(pGameStatus)
		, m_hasNegativeScrollSpeed(pHighwayScroll->hasNegativeScrollSpeed())
	{
	}

	HighwayScrollContext::~HighwayScrollContext() = default;

	int32 HighwayScrollContext::getPositionY(kson::Pulse pulse) const
	{
		return m_pHighwayScroll->getPositionY(pulse, *m_pBeatInfo, *m_pTimingCache, *m_pGameStatus);
	}

	bool HighwayScrollContext::isPulseRangeInDrawRange(kson::Pulse startPulse, kson::Pulse endPulse, int32 positionYOffset) const
	{
		if (endPulse < startPulse)
		{
			std::swap(startPulse, endPulse);
		}

		int32 minY = getPositionY(startPulse) + positionYOffset;
		int32 maxY = minY;

		const auto addPositionY = [&](kson::Pulse pulse)
		{
			const int32 positionY = getPositionY(pulse) + positionYOffset;
			minY = Min(minY, positionY);
			maxY = Max(maxY, positionY);
		};

		if (m_hasNegativeScrollSpeed)
		{
			for (kson::Pulse pulse = startPulse + kDrawRangeSampleInterval; pulse < endPulse; pulse += kDrawRangeSampleInterval)
			{
				addPositionY(pulse);
			}
		}

		addPositionY(endPulse);

		return maxY >= 0 && minY < Graphics::kHighwayTextureSize.y;
	}

	int32 HighwayScrollContext::relPulseToPixelHeight(kson::Pulse basePulse, kson::RelPulse relPulse) const
	{
		return m_pHighwayScroll->relPulseToPixelHeight(basePulse, relPulse, *m_pBeatInfo);
	}

	const HighwayScroll& HighwayScrollContext::highwayScroll() const
	{
		return *m_pHighwayScroll;
	}

	bool HighwayScrollContext::hasNegativeScrollSpeed() const
	{
		return m_hasNegativeScrollSpeed;
	}

	bool HighwayScrollContext::isScrollSpeedPositiveAt(kson::Pulse pulse) const
	{
		// 負のscroll_speedが存在しない場合は常に正
		if (!m_hasNegativeScrollSpeed)
		{
			return true;
		}

		return kson::GraphValueAt(m_pBeatInfo->scrollSpeed, pulse) >= 0.0;
	}

	int32 HighwayScrollContext::currentHispeedWithScrollSpeed() const
	{
		const int32 hispeed = m_pHighwayScroll->currentHispeed();

		// CModはscroll_speedを使わないのでそのまま返す
		if (m_pHighwayScroll->hispeedSetting().type == HispeedType::CMod)
		{
			return hispeed;
		}

		// scroll_speedが空の場合はscroll_speedが1.0とみなす
		if (m_pBeatInfo->scrollSpeed.empty())
		{
			return hispeed;
		}

		// 譜面停止中は0ではなく元のハイスピード値を採用
		const kson::Pulse currentPulse = m_pGameStatus->currentPulse;
		if (!m_pBeatInfo->stop.empty())
		{
			const auto it = m_pBeatInfo->stop.upper_bound(currentPulse);
			if (it != m_pBeatInfo->stop.begin())
			{
				const auto prevIt = std::prev(it);
				if (currentPulse < prevIt->first + prevIt->second)
				{
					return hispeed;
				}
			}
		}

		const double scrollSpeed = kson::GraphValueAt(m_pBeatInfo->scrollSpeed, currentPulse);
		return static_cast<int32>(hispeed * Abs(scrollSpeed));
	}

	int32 HighwayScrollContext::hispeedWithScrollSpeedAt(kson::Pulse pulse) const
	{
		const int32 hispeed = m_pHighwayScroll->hispeedAt(pulse, *m_pBeatInfo);

		if (m_pHighwayScroll->hispeedSetting().type == HispeedType::CMod)
		{
			// CModはscroll_speedを無視するのでそのまま返す
			return hispeed;
		}

		if (m_pBeatInfo->scrollSpeed.empty())
		{
			// scroll_speedがなければ常に等速
			return hispeed;
		}

		const double scrollSpeed = kson::GraphValueAt(m_pBeatInfo->scrollSpeed, pulse);
		return static_cast<int32>(hispeed * Abs(scrollSpeed));
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
			// 現在地点からノーツ地点までの区間を積分計算(scrollSpeedが空なら単純な差分になる)
			return scrollSpeedIntegral(beatInfo.scrollSpeed, gameStatus.currentPulseDouble, static_cast<double>(pulse));
		}
	}

	double HighwayScroll::scrollSpeedIntegral(const kson::Graph& scrollSpeed, double startPulse, double endPulse) const
	{
		if (scrollSpeed.empty())
		{
			return endPulse - startPulse;
		}

		if (endPulse < startPulse)
		{
			return -scrollSpeedIntegral(scrollSpeed, endPulse, startPulse);
		}

		// 区間の分割位置となるグラフ点の範囲を求める
		const auto beginItr = std::upper_bound(m_scrollSpeedPulses.begin(), m_scrollSpeedPulses.end(), static_cast<kson::Pulse>(std::floor(startPulse)));
		const auto endItr = std::lower_bound(m_scrollSpeedPulses.begin(), m_scrollSpeedPulses.end(), endPulse);

		if (beginItr >= endItr)
		{
			// 間にグラフ点がない場合は1区間として計算
			return CalcScrollSpeedSegmentIntegral(scrollSpeed, startPulse, endPulse);
		}

		const size_t beginIdx = static_cast<size_t>(beginItr - m_scrollSpeedPulses.begin());
		const size_t lastIdx = static_cast<size_t>(endItr - m_scrollSpeedPulses.begin()) - 1;

		// 端数の2区間のみ台形則で計算し、グラフ点間は事前計算した累積値の差分で求める
		const double headIntegral = CalcScrollSpeedSegmentIntegral(scrollSpeed, startPulse, static_cast<double>(m_scrollSpeedPulses[beginIdx]));
		const double bodyIntegral = m_scrollSpeedIntegralPrefixSums[lastIdx] - m_scrollSpeedIntegralPrefixSums[beginIdx];
		const double tailIntegral = CalcScrollSpeedSegmentIntegral(scrollSpeed, static_cast<double>(m_scrollSpeedPulses[lastIdx]), endPulse);

		return headIntegral + bodyIntegral + tailIntegral;
	}

	HighwayScroll::HighwayScroll(const kson::ChartData& chartData)
		: m_stdBPM(kson::GetEffectiveStdBPM(chartData))
		, m_hasNegativeScrollSpeed(HasNegativeScrollSpeed(chartData.beat))
	{
		// scrollSpeedのグラフ点間の積分値を累積したテーブルを作成
		const kson::Graph& scrollSpeed = chartData.beat.scrollSpeed;
		m_scrollSpeedPulses.reserve(scrollSpeed.size());
		m_scrollSpeedIntegralPrefixSums.reserve(scrollSpeed.size());
		double prefixSum = 0.0;
		for (const auto& [pulse, graphValue] : scrollSpeed)
		{
			if (!m_scrollSpeedPulses.isEmpty())
			{
				prefixSum += CalcScrollSpeedSegmentIntegral(scrollSpeed, static_cast<double>(m_scrollSpeedPulses.back()), static_cast<double>(pulse));
			}
			m_scrollSpeedPulses.push_back(pulse);
			m_scrollSpeedIntegralPrefixSums.push_back(prefixSum);
		}
	}

	void HighwayScroll::update(const HispeedSetting& hispeedSetting, double currentBPM)
	{
		m_hispeedSetting = hispeedSetting;
		m_currentBPM = currentBPM;
		m_hispeedFactor = HispeedFactor(hispeedSetting, m_stdBPM);
		m_currentHispeed = CurrentHispeed(hispeedSetting, currentBPM, m_stdBPM);
	}

	int32 HighwayScroll::getPositionY(kson::Pulse pulse, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache, const GameStatus& gameStatus) const
	{
		assert(m_hispeedFactor != 0.0 && "HighwayScroll::update() must be called at least once before HighwayScroll::getPositionY()");

		const double relPulseEquivalent = getRelPulseEquvalent(pulse, beatInfo, timingCache, gameStatus);
		return static_cast<int32>(Graphics::kHighwayTextureSize.y - Graphics::kJdglineYFromBottom) - static_cast<int32>(relPulseEquivalent * kBasePixels * m_hispeedFactor / kson::kResolution4);
	}

	int32 HighwayScroll::relPulseToPixelHeight(kson::Pulse basePulse, kson::RelPulse relPulse, const kson::BeatInfo& beatInfo) const
	{
		assert(m_hispeedFactor != 0.0 && "HighwayScroll::update() must be called at least once before HighwayScroll::relPulseToPixelHeight()");

		const double relPulseEquivalent = scrollSpeedIntegral(beatInfo.scrollSpeed, static_cast<double>(basePulse), static_cast<double>(basePulse + relPulse));
		return static_cast<int32>(relPulseEquivalent * kBasePixels * m_hispeedFactor / kson::kResolution4);
	}

	bool HighwayScroll::hasNegativeScrollSpeed() const
	{
		return m_hasNegativeScrollSpeed;
	}

	const HispeedSetting& HighwayScroll::hispeedSetting() const
	{
		return m_hispeedSetting;
	}

	int32 HighwayScroll::currentHispeed() const
	{
		return m_currentHispeed;
	}

	int32 HighwayScroll::hispeedAt(kson::Pulse pulse, const kson::BeatInfo& beatInfo) const
	{
		return CurrentHispeed(m_hispeedSetting, kson::TempoAt(pulse, beatInfo), m_stdBPM);
	}
}
