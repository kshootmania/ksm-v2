#pragma once
#include "Course/CourseInfo.hpp"
#include "Course/CoursePlayResult.hpp"
#include "Course/CourseContinuation.hpp"
#include "MusicGame/PlayResult.hpp"
#include "HighScore/KscKey.hpp"

class CoursePlayState
{
private:
	CourseInfo m_courseInfo;
	size_t m_currentChartIdx = 0U;
	Array<MusicGame::PlayResult> m_results;

	int32 m_totalScore = 0;
	int32 m_totalCritical = 0;
	int32 m_totalNear = 0;
	int32 m_totalError = 0;

	CourseContinuation m_continuation; // 次の曲に引き継ぐ情報

	bool m_cleared = false; // コースクリア判定

	double m_failedChartTimeProgress = 0.0; // 失敗した曲での進行率(0.0〜1.0)

	KscKey m_kscKey;

public:
	CoursePlayState() = default;

	explicit CoursePlayState(const CourseInfo& courseInfo, const KscKey& kscKey)
		: m_courseInfo(courseInfo)
		, m_results(courseInfo.chartCount())
		, m_kscKey(kscKey)
	{
	}

	void recordResult(const MusicGame::PlayResult& playResult)
	{
		if (m_currentChartIdx >= m_results.size())
		{
			return;
		}

		m_results[m_currentChartIdx] = playResult;

		m_totalScore += playResult.score;
		m_totalCritical += playResult.comboStats.critical;
		m_totalNear += playResult.comboStats.totalNear();
		m_totalError += playResult.comboStats.error;

		m_failedChartTimeProgress = playResult.chartTimeProgress;

		// 次の曲に引き継ぐ情報を更新
		m_continuation.gaugeValue = playResult.gaugeValue;
		m_continuation.combo = playResult.finalCourseCombo;
		m_continuation.isNoError = m_continuation.isNoError && playResult.comboStats.error == 0;
	}

	[[nodiscard]]
	bool hasNextChart() const
	{
		return m_currentChartIdx + 1 < m_courseInfo.charts.size();
	}

	void advanceToNextChart()
	{
		if (hasNextChart())
		{
			++m_currentChartIdx;
		}
	}

	[[nodiscard]]
	FilePath currentChartPath() const
	{
		if (m_currentChartIdx >= m_courseInfo.charts.size())
		{
			return U"";
		}
		return m_courseInfo.charts[m_currentChartIdx].absolutePath;
	}

	[[nodiscard]]
	size_t currentChartIdx() const
	{
		return m_currentChartIdx;
	}

	[[nodiscard]]
	int32 totalChartCount() const
	{
		return static_cast<int32>(m_courseInfo.charts.size());
	}

	[[nodiscard]]
	const CourseInfo& courseInfo() const
	{
		return m_courseInfo;
	}

	[[nodiscard]]
	const Array<MusicGame::PlayResult>& results() const
	{
		return m_results;
	}

	[[nodiscard]]
	int32 totalScore() const
	{
		return m_totalScore;
	}

	[[nodiscard]]
	int32 achievementRate() const
	{
		if (isAborted())
		{
			return 0;
		}

		const int32 chartCount = static_cast<int32>(m_courseInfo.charts.size());
		const int32 avgScore = chartCount > 0 ? m_totalScore / chartCount : 0;

		if (m_cleared)
		{
			constexpr int32 kScoreThreshold = MusicGame::kScoreMax * 9 / 10;
			constexpr int32 kScoreDivisor = 10000;
			return 100 + Max(avgScore - kScoreThreshold, 0) / kScoreDivisor;
		}
		else
		{
			if (chartCount <= 0)
			{
				return 0;
			}

			const double completedRatio = static_cast<double>(m_currentChartIdx) / chartCount;
			const double failedChartRatio = m_failedChartTimeProgress / chartCount;
			return static_cast<int32>((completedRatio + failedChartRatio) * 100.0);
		}
	}

	[[nodiscard]]
	int32 totalCritical() const
	{
		return m_totalCritical;
	}

	[[nodiscard]]
	int32 totalNear() const
	{
		return m_totalNear;
	}

	[[nodiscard]]
	int32 totalError() const
	{
		return m_totalError;
	}

	[[nodiscard]]
	bool isAborted() const
	{
		// プレイした曲のみチェック
		for (size_t i = 0; i <= m_currentChartIdx && i < m_results.size(); ++i)
		{
			if (m_results[i].isAborted)
			{
				return true;
			}
		}
		return false;
	}

	void setCleared(bool cleared)
	{
		m_cleared = cleared;
	}

	[[nodiscard]]
	bool isCleared() const
	{
		return m_cleared;
	}

	[[nodiscard]]
	const CourseContinuation& continuation() const
	{
		return m_continuation;
	}

	[[nodiscard]]
	const KscKey& kscKey() const
	{
		return m_kscKey;
	}

	[[nodiscard]]
	CoursePlayResult coursePlayResult() const;
};
