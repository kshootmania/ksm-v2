#pragma once

namespace MusicGame::Judgment
{
	struct ComboStatus
	{
	private:
		// コンボ数
		int32 m_combo = 0;

		// ERROR判定を一度も出していない状態かどうか
		bool m_isNoError = true;

	public:
		/// @brief コンボ加算
		void increment();

		/// @brief ERROR判定によるコンボ数リセット
		void resetByErrorJudgment();

		/// @brief コンボ数を取得
		int32 combo() const;

		/// @brief ERROR判定を一度も出していない状態かどうかを取得
		bool isNoError() const;
	};
}
