#include "HighwayScroll.hpp"

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

		/// @brief scrollSpeedを考慮したノーツの相対Pulse値を計算
		/// @param notePulse ノーツのPulse位置
		/// @param currentPulseDouble 現在のPulse位置
		/// @param scrollSpeed scrollSpeedグラフ
		/// @return scrollSpeedを考慮した相対Pulse値
		double CalcScrollSpeedAdjustedRelPulse(kson::Pulse notePulse, double currentPulseDouble, const kson::Graph& scrollSpeed)
		{
			if (scrollSpeed.empty())
			{
				return static_cast<double>(notePulse - currentPulseDouble);
			}

			const kson::Pulse currentPulse = static_cast<kson::Pulse>(currentPulseDouble);

			// notePulseが現在位置より未来の場合
			if (notePulse > currentPulse)
			{
				double totalRelPulse = 0.0;
				kson::Pulse segmentStartPulse = currentPulse;
				double currentSpeed = kson::GraphValueAt(scrollSpeed, segmentStartPulse);

				while (segmentStartPulse < notePulse)
				{
					auto nextItr = scrollSpeed.upper_bound(segmentStartPulse);

					kson::Pulse segmentEndPulse;
					double nextSpeed;
					if (nextItr != scrollSpeed.end() && nextItr->first <= notePulse)
					{
						segmentEndPulse = nextItr->first;
						nextSpeed = nextItr->second.v.v;
					}
					else
					{
						segmentEndPulse = notePulse;
						nextSpeed = currentSpeed;
					}

					// 台形則で積分
					const kson::Pulse length = segmentEndPulse - segmentStartPulse;
					totalRelPulse += length * (currentSpeed + nextSpeed) / 2.0;

					// 次の区間の開始速度を設定
					if (nextItr != scrollSpeed.end() && nextItr->first == segmentEndPulse)
					{
						currentSpeed = nextItr->second.v.vf;
					}
					else
					{
						currentSpeed = nextSpeed;
					}

					segmentStartPulse = segmentEndPulse;
				}

				return totalRelPulse;
			}
			// notePulseが現在位置より過去の場合
			else
			{
				double totalRelPulse = 0.0;
				kson::Pulse segmentEndPulse = currentPulse;
				double endSpeed = kson::GraphValueAt(scrollSpeed, segmentEndPulse);

				while (segmentEndPulse > notePulse)
				{
					// segmentEndPulse未満で最大のscroll_speed変更点を探す
					auto itr = scrollSpeed.lower_bound(segmentEndPulse);
					if (itr != scrollSpeed.begin())
					{
						--itr;
					}

					kson::Pulse segmentStartPulse;
					double startSpeed;
					if (itr == scrollSpeed.begin() && itr->first > notePulse)
					{
						// scroll_speedの最初の変更点より前の区間
						segmentStartPulse = notePulse;
						startSpeed = itr->second.v.v;
					}
					else if (itr != scrollSpeed.end() && itr->first > notePulse)
					{
						segmentStartPulse = itr->first;
						startSpeed = itr->second.v.v;
					}
					else
					{
						segmentStartPulse = notePulse;
						startSpeed = endSpeed;
					}

					// 台形則で積分(負の方向)
					const kson::Pulse length = segmentEndPulse - segmentStartPulse;
					totalRelPulse -= length * (startSpeed + endSpeed) / 2.0;

					// 次の区間へ移動
					// 次の区間の終了速度を設定(scroll_speed変更点がある場合はその直前の速度)
					if (itr != scrollSpeed.end() && itr->first == segmentStartPulse)
					{
						// itrがbeginの場合、これ以上前に戻れないのでループ終了
						if (itr == scrollSpeed.begin())
						{
							break;
						}
						// 一つ前のscroll_speed変更点の終了速度を取得
						auto prevItr = std::prev(itr);
						endSpeed = prevItr->second.v.vf;
					}
					else
					{
						endSpeed = startSpeed;
					}

					segmentEndPulse = segmentStartPulse;
				}

				return totalRelPulse;
			}
		}
	}

	HighwayScrollContext::HighwayScrollContext(const HighwayScroll* pHighwayScroll, const kson::BeatInfo* pBeatInfo, const kson::TimingCache* pTimingCache, const GameStatus* pGameStatus)
		: m_pHighwayScroll(pHighwayScroll)
		, m_pBeatInfo(pBeatInfo)
		, m_pTimingCache(pTimingCache)
		, m_pGameStatus(pGameStatus)
		, m_hasNegativeScrollSpeed(HasNegativeScrollSpeed(*pBeatInfo))
	{
	}

	HighwayScrollContext::~HighwayScrollContext() = default;

	int32 HighwayScrollContext::getPositionY(kson::Pulse pulse) const
	{
		return m_pHighwayScroll->getPositionY(pulse, *m_pBeatInfo, *m_pTimingCache, *m_pGameStatus);
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

		const double scrollSpeed = kson::GraphValueAt(m_pBeatInfo->scrollSpeed, m_pGameStatus->currentPulse);
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
			// scrollSpeedがなければ単純な差分計算
			if (beatInfo.scrollSpeed.empty())
			{
				return static_cast<double>(pulse) - gameStatus.currentPulseDouble;
			}

			// scrollSpeedがあれば現在地点からノーツ地点までの区間を積分計算
			return CalcScrollSpeedAdjustedRelPulse(pulse, gameStatus.currentPulseDouble, beatInfo.scrollSpeed);
		}
	}

	HighwayScroll::HighwayScroll(const kson::ChartData& chartData)
		: m_stdBPM(kson::GetEffectiveStdBPM(chartData))
	{
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

		const double relPulseEquivalent = CalcScrollSpeedAdjustedRelPulse(basePulse + relPulse, static_cast<double>(basePulse), beatInfo.scrollSpeed);
		return static_cast<int32>(relPulseEquivalent * kBasePixels * m_hispeedFactor / kson::kResolution4);
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
