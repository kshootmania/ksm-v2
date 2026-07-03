#pragma once

/// @brief クリック長押しでの連続発火を管理するクラス
class ClickHoldRepeat
{
private:
	const double m_intervalSec;
	const double m_intervalSecFirst;
	Stopwatch m_pressedTimeStopwatch;
	double m_pressedTimeSecPrev = 0.0;

public:
	/// @brief コンストラクタ
	/// @param intervalSec 2回目以降の発火間隔(秒)
	/// @param intervalSecFirst 1回目から2回目までの発火間隔(0の場合、intervalSecと同じ値に設定される)
	explicit ClickHoldRepeat(double intervalSec, double intervalSecFirst);

	/// @brief フレーム毎の更新
	/// @param pressed 押下中かどうか
	/// @return 現在フレームでの発火回数(押下開始フレームでは1)
	int32 update(bool pressed);
};
