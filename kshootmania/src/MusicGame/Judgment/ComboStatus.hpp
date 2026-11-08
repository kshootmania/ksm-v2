#pragma once
#undef near // Windows.h対策
#include "JudgmentDefines.hpp"
#include "Course/CourseContinuation.hpp"

namespace MusicGame::Judgment
{
	struct ComboStats
	{
		int32 critical = 0;

		int32 nearFast = 0;

		int32 nearSlow = 0;

		int32 error = 0;

		int32 totalNear() const
		{
			return nearFast + nearSlow;
		}

		int32 totalJudgedCombo() const
		{
			return critical + totalNear() + error;
		}
	};

	struct ComboStatus
	{
	private:
		/// @brief 現在のコンボ数
		int32 m_combo = 0;

		/// @brief 最大コンボ数
		int32 m_maxCombo = 0;

		/// @brief コース全体でのコンボ数(コースモード時のみ使用)
		int32 m_courseCombo = 0;

		/// @brief コース全体での最大コンボ数(コースモード時のみ使用)
		int32 m_courseMaxCombo = 0;

		/// @brief コース全体でのエラーなし状態(コースモード時のみ使用)
		bool m_courseIsNoError = true;

		/// @brief 判定毎のコンボ数の内訳
		ComboStats m_stats;

	public:
		explicit ComboStatus(const Optional<CourseContinuation>& courseContinuation)
			: m_courseCombo(courseContinuation.has_value() ? courseContinuation->combo : 0)
			, m_courseMaxCombo(m_courseCombo)
			, m_courseIsNoError(courseContinuation.has_value() ? courseContinuation->isNoError : true)
		{
		}

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

		/// @brief コース全体でのコンボ数を取得(コースモード時のみ使用)
		/// @return コンボ数
		int32 courseCombo() const;

		/// @brief コース全体での最大コンボ数を取得(コースモード時のみ使用)
		/// @return 最大コンボ数
		int32 courseMaxCombo() const;

		/// @brief コース全体でのエラーなし状態を取得(コースモード時のみ使用)
		/// @return エラーなし状態
		bool courseIsNoError() const;
	};
}
