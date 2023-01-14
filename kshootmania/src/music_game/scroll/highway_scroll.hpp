#pragma once
#include <mutex>
#include "music_game/game_status.hpp"
#include "music_game/graphics/graphics_defines.hpp"
#include "hispeed_setting.hpp"
#include "kson/kson.hpp"

namespace MusicGame::Scroll
{
	class HighwayScroll;

	/// @brief HighwayScrollのコンテキスト
	class HighwayScrollContext
	{
	private:
		const HighwayScroll* const m_pHighwayScroll;
		const kson::BeatInfo* const m_pBeatInfo;
		const kson::TimingCache* const m_pTimingCache;
		const GameStatus* const m_pGameStatus;

	public:
		/// @brief コンストラクタ
		/// @param pHighwayScroll HighwayScrollのポインタ(メンバ関数呼出時点で有効なポインタであること)
		/// @param pBeatInfo kson.beatのポインタ(メンバ関数呼出時点で有効なポインタであること)
		/// @param pTimingCache TimingCacheのポインタ(メンバ関数呼出時点で有効なポインタであること)
		/// @param pGameStatus GameStatusのポインタ(メンバ関数呼出時点で有効なポインタであること)
		explicit HighwayScrollContext(const HighwayScroll* pHighwayScroll, const kson::BeatInfo* pBeatInfo, const kson::TimingCache* pTimingCache, const GameStatus* pGameStatus);

		~HighwayScrollContext();

		/// @brief Pulse値をもとにHighway上のY座標を求める
		/// @param pulse Pulse値
		/// @return Y座標
		int32 getPositionY(kson::Pulse pulse) const;

		/// @brief HighwayScrollへの参照を返す
		/// @return HighwayScrollへの参照
		const HighwayScroll& highwayScroll() const;
	};

	/// @brief Highway上のスクロール計算(ハイスピードおよびscroll_speedの計算)
	class HighwayScroll
	{
	private:
		/// @brief o-mod用の基準BPM
		const double m_stdBPM;

		/// @brief ハイスピード設定
		HispeedSetting m_hispeedSetting;

		/// @brief 現在のハイスピード値
		double m_currentBPM = 120.0;

		/// @brief ハイスピード係数
		double m_hispeedFactor = 0.0;

		/// @brief 現在のハイスピード値
		int32 m_currentHispeed = 0;

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
		double getRelPulseEquvalent(kson::Pulse pulse, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache, const GameStatus& gameStatus) const;

	public:
		explicit HighwayScroll(const kson::ChartData& chartData);

		/// @brief 毎フレームの更新
		/// @param hispeedSetting ハイスピード設定
		/// @param currentBPM 現在のBPM
		void update(const HispeedSetting& hispeedSetting, double currentBPM);

		/// @brief Pulse値をもとにHighway上のY座標を求める
		/// @param pulse Pulse値
		/// @param beatInfo kson.beat
		/// @param timingCache 事前計算したTimingCache
		/// @param gameStatus ゲーム状態
		/// @param hispeedSetting ハイスピード設定
		/// @return Y座標
		int32 getPositionY(kson::Pulse pulse, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache, const GameStatus& gameStatus) const;

		/// @brief ハイスピード設定を返す
		/// @return ハイスピード設定
		const HispeedSetting& hispeedSetting() const;

		/// @brief 現在のハイスピード値を返す
		/// @return ハイスピード値
		int32 currentHispeed() const;

		/// @brief 現在のハイスピード値に最も近いハイスピード値が得られるハイスピード設定値を返す(ハイスピードの種類変更時に前の種類での値に最も近い設定にするために使用)
		/// @param targetHispeedType 対象とするハイスピードの種類
		/// @return ハイスピード設定値
		/// @remarks 戻り値は範囲外の値を丸めたり25刻みにしたりしないので、使用側で適用すること
		int32 nearestHispeedSettingValue(HispeedType targetHispeedType) const;
	};
}
