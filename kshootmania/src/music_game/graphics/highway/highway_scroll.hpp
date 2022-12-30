#pragma once
#include <mutex>
#include "music_game/game_status.hpp"
#include "music_game/hispeed_setting.hpp"
#include "music_game/graphics/graphics_defines.hpp"
#include "kson/kson.hpp"

namespace MusicGame::Graphics
{
	/// @brief Highway上のスクロール計算(ハイスピードおよびscroll_speedの計算)
	class HighwayScroll
	{
	private:
		/// @brief o-mod用の基準BPM
		const double m_stdBPM;

		/// @brief ハイスピード設定
		HispeedSetting m_hispeedSetting;

		/// @brief ハイスピード因数
		double m_hispeedFactor = 0.0;

		/// @brief キャッシュの排他処理用mutex(M&M rule)
		mutable std::mutex m_mutex;

		/// @brief pulseToSecのキャッシュ
		mutable std::unordered_map<kson::Pulse, double> m_pulseToSecCache; // TODO: reserveした方が良さそう。そもそも譜面読み込み時に最初に計算した方が良い

		/// @brief 時間をPulse値から秒数へ変換(キャッシュ付き)
		/// @param pulse Pulse値
		/// @param beatInfo kson.beat
		/// @param timingCache 事前計算したTimingCache
		/// @return 秒数
		/// @note ハイスピードの種類のうちC-modでのみ使用される
		double pulseToSec(kson::Pulse pulse, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache) const;

		/// @brief 現在時間からの相対Pulse数を求める(C-modの場合は秒数をもとに計算した換算値を返す)
		/// @param pulse Pulse値
		/// @param beatInfo kson.beat
		/// @param timingCache 事前計算したTimingCache
		/// @param gameStatus ゲーム状態
		/// @param hispeedSetting ハイスピード設定
		/// @return 相対Pulse数換算値
		/// @note HSP版: https://github.com/m4saka/kshootmania-v1-hsp/blob/1c75880b545d1232eeffc4bb3fc19704a3622f73/src/scene/play/play_utils.hsp#L246-L269
		kson::RelPulse getRelPulseEquvalent(kson::Pulse pulse, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache, const GameStatus& gameStatus) const;

	public:
		explicit HighwayScroll(const kson::ChartData& chartData);

		/// @brief 毎フレームの更新
		/// @param hispeedSetting ハイスピード設定
		void update(const HispeedSetting& hispeedSetting);

		/// @brief Pulse値をもとにHighway上のY座標を求める
		/// @param pulse Pulse値
		/// @param beatInfo kson.beat
		/// @param timingCache 事前計算したTimingCache
		/// @param gameStatus ゲーム状態
		/// @param hispeedSetting ハイスピード設定
		/// @return Y座標
		int32 getPositionY(kson::Pulse pulse, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache, const GameStatus& gameStatus) const;
	};
}
