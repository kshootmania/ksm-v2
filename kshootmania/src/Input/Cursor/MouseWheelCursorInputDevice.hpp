#pragma once
#include "ICursorInputDevice.hpp"

class MouseWheelCursorInputDevice : public ICursorInputDevice
{
private:
	int32 m_deltaCursor = 0;
	double m_accumulatedDelta = 0.0;
	Stopwatch m_idleStopwatch{ StartImmediately::Yes };
	Stopwatch m_continuousInputStopwatch;
	Stopwatch m_emitStopwatch;

public:
	/// @brief コンストラクタ
	MouseWheelCursorInputDevice();

	virtual ~MouseWheelCursorInputDevice();

	/// @brief フレーム毎の更新(毎フレーム、deltaCursor使用前に呼ぶ)
	virtual void update() override;

	/// @brief 現在フレームでのカーソル移動数を返す
	/// @return カーソル移動数(移動していない場合は0)
	virtual int32 deltaCursor() const override;
};
