#pragma once
#include <mutex>
#include "MusicGame/GameStatus.hpp"
#include "MusicGame/Graphics/GraphicsDefines.hpp"
#include "HispeedSetting.hpp"
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

		/// @brief scrollSpeedに負の値が含まれているか(コンストラクタで事前計算)
		const bool m_hasNegativeScrollSpeed;

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

		/// @brief Pulse値の区間がHighwayの描画範囲内にあるかを返す
		/// @param startPulse 開始Pulse値
		/// @param endPulse 終了Pulse値
		/// @param positionYOffset Y座標への加算オフセット
		/// @return 描画範囲内にある場合true
		bool isPulseRangeInDrawRange(kson::Pulse startPulse, kson::Pulse endPulse, int32 positionYOffset = 0) const;

		/// @brief 相対Pulse値をピクセル高さに変換する
		/// @param basePulse 基準となるPulse位置(scroll_speed計算に使用)
		/// @param relPulse 相対Pulse値
		/// @return ピクセル高さ
		int32 relPulseToPixelHeight(kson::Pulse basePulse, kson::RelPulse relPulse) const;

		/// @brief HighwayScrollへの参照を返す
		/// @return HighwayScrollへの参照
		const HighwayScroll& highwayScroll() const;

		/// @brief scrollSpeedに負の値が含まれているかを返す
		/// @return 負の値が含まれていればtrue
		bool hasNegativeScrollSpeed() const;

		/// @brief 指定したPulse位置でのscrollSpeedが正であるかを返す
		/// @param pulse Pulse位置
		/// @return scrollSpeedが0以上ならtrue
		bool isScrollSpeedPositiveAt(kson::Pulse pulse) const;

		/// @brief scroll_speedを考慮した現在のハイスピード値を返す
		/// @return scroll_speed適用済みのハイスピード値(負のscroll_speedは絶対値で計算)
		int32 currentHispeedWithScrollSpeed() const;
	};

	/// @brief Highway上のスクロール計算(ハイスピードおよびscroll_speedの計算)
	class HighwayScroll
	{
	private:
		/// @brief o-mod用の基準BPM
		const double m_stdBPM;

		/// @brief ハイスピード設定
		HispeedSetting m_hispeedSetting;

		/// @brief 現在のBPM
		double m_currentBPM = kDefaultBPM;

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
		/// @note HSP版: https://github.com/kshootmania/ksm-v1/blob/1c75880b545d1232eeffc4bb3fc19704a3622f73/src/scene/play/play_utils.hsp#L246-L269
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

		/// @brief 相対Pulse値をピクセル高さに変換する
		/// @param basePulse 基準となるPulse位置(scroll_speed計算に使用)
		/// @param relPulse 相対Pulse値
		/// @param beatInfo kson.beat
		/// @return ピクセル高さ
		int32 relPulseToPixelHeight(kson::Pulse basePulse, kson::RelPulse relPulse, const kson::BeatInfo& beatInfo) const;

		/// @brief ハイスピード設定を返す
		/// @return ハイスピード設定
		const HispeedSetting& hispeedSetting() const;

		/// @brief 現在のハイスピード値を返す
		/// @return ハイスピード値
		int32 currentHispeed() const;
	};
}
