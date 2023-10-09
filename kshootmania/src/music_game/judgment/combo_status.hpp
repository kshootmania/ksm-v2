#pragma once
#include "judgment_defines.hpp"

namespace MusicGame::Judgment
{
	struct ComboStats
	{
		int32 critical = 0;

		int32 nearFast = 0;

		int32 nearSlow = 0;

		int32 error = 0;

		int32 near() const
		{
			return nearFast + nearSlow;
		}
	};

	struct ComboStatus
	{
	private:
		/// @brief コンボ数
		int32 m_combo = 0;

		/// @brief 最大コンボ数
		int32 m_maxCombo = 0;

		/// @brief 判定毎のコンボ数の内訳
		ComboStats m_stats;

	public:
		void processJudgmentResult(JudgmentResult result);

		/// @brief コンボ数を取得
		int32 combo() const;

		/// @brief 最大コンボ数を取得
		int32 maxCombo() const;

		/// @brief 判定毎のコンボ数の内訳を取得
		const ComboStats& stats() const;

		/// @brief ERROR判定を一度も出していない状態かどうかを取得
		bool isNoError() const;
	};
}
