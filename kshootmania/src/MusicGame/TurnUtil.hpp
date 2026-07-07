#pragma once
#include "GameDefines.hpp"
#include "kson/ChartData.hpp"

namespace MusicGame
{
	// TurnTableを譜面データに適用
	void ApplyTurnTable(kson::ChartData& chartData, const TurnTable& turnTable);

	/// @brief S-RANDOM用にBT・FX全ノーツの配置先レーンを生成
	/// @param playbackSpeed 再生速度(縦連回避を秒数ベースで行うために使用)
	/// @return 正規譜面の各ノーツ(pulse昇順、同pulse内はレーン昇順、レーンは0〜3がBT、4〜5がFX)に対応する配置先レーン(0〜5)
	[[nodiscard]]
	Array<std::size_t> MakeSRandomNoteLanes(const kson::ChartData& chartData, double playbackSpeed);

	/// @brief S-RANDOMによる配置変更をBT・FXノーツに適用
	/// @return 配置結果に不整合がなければtrue
	/// @remark 音声エフェクト・キー音は元のFXレーン基準のまま処理するため変更しない
	bool ApplySRandomNoteLanes(kson::ChartData* pChartData, const Array<std::size_t>& noteLanes);

	/// @brief 元のFXレーンにおける各ノーツの配置先レーン(0〜5)を取得
	/// @param chartData 配置適用前の譜面データ
	/// @param noteLanes 配置先レーン
	[[nodiscard]]
	std::array<kson::ByPulse<std::size_t>, kson::kNumFXLanesSZ> MakeFXNoteDestinations(const kson::ChartData& chartData, const Array<std::size_t>& noteLanes);
}
