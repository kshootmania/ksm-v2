#pragma once
#undef near // Windows.h対策
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

		int32 totalJudgedCombo() const
		{
			return critical + near() + error;
		}
	};

	struct ComboStatus
	{
	private:
		/// @brief 現在のコンボ数
		int32 m_combo = 0;

		/// @brief 最大コンボ数
		int32 m_maxCombo = 0;

		/// @brief 判定毎のコンボ数の内訳
		ComboStats m_stats;

	public:
		void processJudgmentResult(JudgmentResult result);

		/// @brief 現在のコンボ数を取得
		/// @return 現在のコンボ数
		int32 combo() const;

		/// @brief 最大コンボ数を取得
		/// @return 最大コンボ数
		int32 maxCombo() const;

		/// @brief 判定毎のコンボ数の内訳を取得
		/// @return 判定毎のコンボ数の内訳
		const ComboStats& stats() const;

		/// @brief ERROR判定を一度も出していない状態かどうかを取得
		/// @return ERROR判定を一度も出していない状態かどうか
		bool isNoError() const;

		/// @brief 判定済みのコンボ数(ERROR判定含む)を取得
		/// @return コンボ数
		int32 totalJudgedCombo() const;
	};
}
