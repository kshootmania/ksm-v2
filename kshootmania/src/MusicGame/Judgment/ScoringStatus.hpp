#pragma once
#include "MusicGame/GameDefines.hpp"
#include "JudgmentDefines.hpp"
#include "ComboStatus.hpp"

namespace MusicGame::Judgment
{
	class ScoringStatus
	{
	private:
		const int32 m_scoreValueMax = 0;
		const int32 m_gaugeValueMax = 0;
		const GaugeType m_gaugeType = GaugeType::kNormalGauge;
		const GameMode m_gameMode = GameMode::kNormal;
		const GaugeCalcType m_gaugeCalcType;

		int32 m_scoreValue = 0;
		int32 m_gaugeValue;
		int32 m_uncappedGaugeValue;
		int32 m_gaugeValueNormal = 0; // NORMAL基準ゲージ値(HARDのGrade計算用)

		ComboStatus m_comboStatus;

		ComboStats m_chipOrLaserSlamStats;

		ComboStats m_longOrLaserLineStats;

		void addGaugeValue(int32 add);

		void addGaugeValueNormal(int32 add);

		/// @brief 実ゲージのみ減らす
		/// @param sub 減少量(EASY/NORMALでは減少量そのもの、HARDでは固定値、コースでは基本値)
		void subtractGaugeValue(int32 sub);

		void subtractGaugeValueNormal(int32 sub);

		/// @brief ERROR時のゲージ減少
		/// @param normalPercent NORMALゲージでのゲージ最大値に対する減少割合(%)
		/// @param hardSub HARDゲージでの固定減少値
		/// @param courseBaseSub コースモードでの減少基本値
		/// @remark Grade計算用のNORMAL基準ゲージはゲージ種類・モードによらずNORMALゲージと同じ量を減らす
		void subtractGaugeValueOnError(double normalPercent, int32 hardSub, int32 courseBaseSub);

		double calcGaugePercentageFromValue(int32 gaugeValue, GaugeType gaugeType) const;

	public:
		ScoringStatus(int32 scoreValueMax, int32 gaugeValueMax, GaugeType gaugeType, const Optional<CourseContinuation>& courseContinuation, GameMode gameMode);

		void onChipOrLaserSlamJudgment(Judgment::JudgmentResult result);

		void onLongOrLaserLineJudgment(Judgment::JudgmentResult result);

		int32 score() const;

		/// @brief ゲージのパーセンテージを返す
		/// @return パーセンテージ(0.0～100.0)
		double gaugePercentage(GaugeType gaugeType) const;

		/// @brief ゲージのパーセンテージを整数で返す(切り捨て)
		/// @return パーセンテージ(0～100)
		int32 gaugePercentageInt(GaugeType gaugeType) const;

		/// @brief Grade計算用のゲージパーセンテージを返す
		/// @return パーセンテージ(0.0～100.0)
		double gaugePercentageForGrade() const;

		/// @brief 内部ゲージ値を返す(コースモード時の引き継ぎ用)
		/// @return 内部ゲージ値
		int32 gaugeValue() const;

		double uncappedGaugePercentage() const;

		int32 uncappedGaugeValue() const;

		int32 combo() const;

		int32 maxCombo() const;

		const ComboStats& comboStats() const;

		const ComboStats& chipOrLaserSlamStats() const;

		const ComboStats& longOrLaserLineStats() const;

		bool isNoError() const;

		int32 totalJudgedCombo() const;

		int32 courseCombo() const;

		int32 courseMaxCombo() const;

		bool courseIsNoError() const;

		/// @brief 表示用のコンボ数を返す
		/// @return コンボ数(コースモード時はコース全体のコンボ数、通常時は楽曲内のコンボ数)
		int32 displayCombo() const;

		/// @brief 表示用のエラーなし状態を返す
		/// @return エラーなし状態(コースモード時はコース全体のエラーなし状態、通常時は楽曲内のエラーなし状態)
		bool displayIsNoError() const;
	};
}
