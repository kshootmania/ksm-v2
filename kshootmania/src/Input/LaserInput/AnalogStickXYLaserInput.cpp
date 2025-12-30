#include "AnalogStickXYLaserInput.hpp"
#include <ksmaxis/ksmaxis.hpp>

namespace
{
	// デフォルトの入力感度設定値
	constexpr int32 kDefaultSensitivity = 50;

	// 入力感度のベース係数
	constexpr double kBaseSensitivityScale = 8.0;

	// 微弱な動きを無視するしきい値
	constexpr double kDeadZone = 0.0005;
}

AnalogStickXYLaserInput::AnalogStickXYLaserInput(int32 laneIdx)
	: m_laneIdx(laneIdx)
{
}

double AnalogStickXYLaserInput::getDeltaCursorX([[maybe_unused]] double deltaTimeSec)
{
	// 左右レーザーを入れ替えるかどうか
	const bool swapLR = ConfigIni::GetBool(ConfigIni::Key::kSwapLaserLR, false);

	// 入力感度
	const int32 sensitivity = ConfigIni::GetInt(ConfigIni::Key::kLaserInputSensitivity, kDefaultSensitivity);
	const double sensitivityScale = kBaseSensitivityScale * (sensitivity / static_cast<double>(kDefaultSensitivity));

	// ksmaxisから移動量取得
	const int32 axisIdx = m_laneIdx ^ (swapLR ? 1 : 0);
	const auto deltas = ksmaxis::GetAxisDeltas(ksmaxis::InputMode::kAnalogStick);
	double delta = deltas[axisIdx] * sensitivityScale;

	// 微弱な動きを無視
	if (Abs(delta) < kDeadZone)
	{
		return 0.0;
	}

	return delta;
}

bool AnalogStickXYLaserInput::reconstructionNeeded() const
{
	const int32 laserInputType = ConfigIni::GetInt(ConfigIni::Key::kLaserInputType, ConfigIni::Value::LaserInputType::kKeyboard);
	return laserInputType != ConfigIni::Value::LaserInputType::kAnalogStickXY;
}
