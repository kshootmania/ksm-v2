#pragma once
#include "icursor_input_device.hpp"

class ButtonCursorInputDevice : public ICursorInputDevice
{
private:
	const Array<KeyConfig::Button> m_incrementButtons;
	const Array<KeyConfig::Button> m_decrementButtons;
	const double m_intervalSec;
	const double m_intervalSecFirst;
	const StartRequiredForBTFXLaser m_startRequiredForBTFXLaser;
	int32 m_deltaCursor = 0;
	Optional<Stopwatch> m_pressedTimeStopwatch;
	double m_pressedTimeSecPrev = 0.0;

public:
	/// @brief コンストラクタ
	/// @param incrementButtons 正の方向のボタン配列
	/// @param decrementButtons 負の方向のボタン配列
	/// @param intervalSec カーソル移動の所要押下時間(0の場合、押下し続けても連続でカーソル移動しない)
	/// @param intervalSecFirst 1回目のカーソル移動までの所要押下時間(0の場合、intervalSecと同じ値に設定される)
	explicit ButtonCursorInputDevice(
		const Array<KeyConfig::Button>& incrementButtons,
		const Array<KeyConfig::Button>& decrementButtons,
		double intervalSec = 0.0,
		double intervalSecFirst = 0.0,
		StartRequiredForBTFXLaser startRequiredForBTFXLaser = StartRequiredForBTFXLaser::No);

	virtual ~ButtonCursorInputDevice();

	/// @brief フレーム毎の更新(毎フレーム、deltaCursor使用前に呼ぶ)
	virtual void update() override;

	/// @brief 現在フレームでのカーソル移動数を返す
	/// @return カーソル移動数(移動していない場合は0)
	virtual int32 deltaCursor() const override;
};
