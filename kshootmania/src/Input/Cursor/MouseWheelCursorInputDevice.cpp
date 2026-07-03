#include "MouseWheelCursorInputDevice.hpp"

namespace
{
	// ホイール1クリック分のスクロール量
	constexpr double kNotchUnit = 1.0;

	// 累積量の上限
	constexpr double kAccumulatedDeltaMax = 2.0;

	// 入力がない状態が続いた場合にリセットするまでの時間
	constexpr Duration kIdleResetTime = 0.3s;

	double GetEmitIntervalSec(double continuousInputSec)
	{
		// 連続入力時間に応じて発火間隔を短くする
		if (continuousInputSec < 0.3)
		{
			return 0.1;
		}
		if (continuousInputSec < 0.6)
		{
			return 0.05;
		}
		return 0.025;
	}

	bool IsDirectionFlipped(double prev, double current)
	{
		return Sign(prev) != Sign(current) && prev != 0.0 && current != 0.0;
	}
}

MouseWheelCursorInputDevice::MouseWheelCursorInputDevice() = default;

MouseWheelCursorInputDevice::~MouseWheelCursorInputDevice() = default;

void MouseWheelCursorInputDevice::update()
{
	m_deltaCursor = 0;

	const double wheel = Mouse::Wheel();
	if (wheel == 0.0)
	{
		if (m_idleStopwatch.elapsed() > kIdleResetTime)
		{
			// 一定時間入力がなければ累積量と連続入力時間をリセット
			m_accumulatedDelta = 0.0;
			m_continuousInputStopwatch.reset();
			m_emitStopwatch.reset();
		}
		return;
	}
	m_idleStopwatch.restart();

	if (!m_continuousInputStopwatch.isRunning())
	{
		m_continuousInputStopwatch.restart();
	}

	// 回転方向が変わったら累積量と連続入力時間をリセット
	if (IsDirectionFlipped(m_accumulatedDelta, wheel))
	{
		m_accumulatedDelta = 0.0;
		m_continuousInputStopwatch.restart();
	}
	m_accumulatedDelta = Clamp(m_accumulatedDelta + wheel, -kAccumulatedDeltaMax, kAccumulatedDeltaMax);

	// 累積量をもとに移動段階数を決める
	int32 steps = static_cast<int32>(m_accumulatedDelta / kNotchUnit);
	if (steps == 0)
	{
		return;
	}

	// 連続入力時間に応じた発火間隔で1回あたりの移動数を制限
	const double emitIntervalSec = GetEmitIntervalSec(m_continuousInputStopwatch.sF());
	const int32 maxSteps = m_emitStopwatch.isRunning() ? static_cast<int32>(m_emitStopwatch.sF() / emitIntervalSec) : 1;
	if (maxSteps <= 0)
	{
		return;
	}
	steps = Clamp(steps, -maxSteps, maxSteps);

	m_deltaCursor = steps;
	m_accumulatedDelta -= steps * kNotchUnit;
	m_emitStopwatch.restart();
}

int32 MouseWheelCursorInputDevice::deltaCursor() const
{
	return m_deltaCursor;
}
