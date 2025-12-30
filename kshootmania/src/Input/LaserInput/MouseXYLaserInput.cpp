#include "MouseXYLaserInput.hpp"
#include <ksmaxis/ksmaxis.hpp>

namespace
{
	// デフォルトの入力感度設定値
	constexpr int32 kDefaultSensitivity = 50;

	// 入力感度のベース係数(マウス用)
	constexpr double kBaseSensitivityScale = 0.01;

	// 微弱な動きを無視するしきい値
	constexpr double kDeadZone = 0.1;
}

MouseXYLaserInput::MouseXYLaserInput(int32 laneIdx)
	: m_laneIdx(laneIdx)
{
}

double MouseXYLaserInput::getDeltaCursorX([[maybe_unused]] double deltaTimeSec)
{
	// 左右レーザーを入れ替えるかどうか
	const bool swapLR = ConfigIni::GetBool(ConfigIni::Key::kSwapLaserLR, false);

	// 入力感度
	const int32 sensitivity = ConfigIni::GetInt(ConfigIni::Key::kLaserInputSensitivity, kDefaultSensitivity);
	const double sensitivityScale = kBaseSensitivityScale * (sensitivity / static_cast<double>(kDefaultSensitivity));

	// ksmaxisからマウス移動量取得
	const auto deltas = ksmaxis::GetAxisDeltas(ksmaxis::InputMode::kMouse);

	// レーン選択(swapLRで入れ替え)
	const int32 axisIdx = m_laneIdx ^ (swapLR ? 1 : 0);

	// 方向設定(X=0, Y=1)
	// directionが1なら通常、0なら反転
	const bool directionX = ConfigIni::GetBool(ConfigIni::Key::kLaserMouseDirectionX, true);
	const bool directionY = ConfigIni::GetBool(ConfigIni::Key::kLaserMouseDirectionY, true);

	// 左LASER(axisIdx=0)はX軸、右LASER(axisIdx=1)はY軸
	double delta;
	if (axisIdx == 0)
	{
		delta = deltas[0] * (directionX ? 1.0 : -1.0);
	}
	else
	{
		delta = deltas[1] * (directionY ? 1.0 : -1.0);
	}

	delta *= sensitivityScale;

	// 微弱な動きを無視
	if (Abs(delta) < kDeadZone * sensitivityScale)
	{
		return 0.0;
	}

	return delta;
}

bool MouseXYLaserInput::reconstructionNeeded() const
{
	const int32 laserInputType = ConfigIni::GetInt(ConfigIni::Key::kLaserInputType, ConfigIni::Value::LaserInputType::kKeyboard);
	return laserInputType != ConfigIni::Value::LaserInputType::kMouseXY;
}
