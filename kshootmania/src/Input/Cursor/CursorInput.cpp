#include "CursorInput.hpp"
#include "ButtonCursorInputDevice.hpp"
#include "Input/KeyConfig.hpp"
#include "Ini/ConfigIni.hpp"

namespace
{
	using Type = CursorInput::Type;

	bool HasFlag(int32 buttonFlags, CursorButtonFlags::CursorButtonFlags flag)
	{
		return (buttonFlags & static_cast<int32>(flag)) != 0;
	}

	bool IsLaserInputDigital()
	{
		const int32 laserInputType = ConfigIni::GetInt(ConfigIni::Key::kLaserInputType, ConfigIni::Value::LaserInputType::kKeyboard);
		return laserInputType == ConfigIni::Value::LaserInputType::kKeyboard;
	}

	Array<Button> IncrementButtonsForHorizontalMenu(int32 buttonFlags, FlipArrowKeyDirectionYN flipArrowKeyDirection)
	{
		Array<Button> incrementButtons;

		if (HasFlag(buttonFlags, CursorButtonFlags::kArrow))
		{
			incrementButtons.push_back(flipArrowKeyDirection ? kButtonLeft : kButtonRight);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBT))
		{
			incrementButtons.push_back(kButtonBT_D);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBTOpposite))
		{
			incrementButtons.push_back(kButtonBT_B);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kFX))
		{
			incrementButtons.push_back(kButtonFX_R);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaser) && IsLaserInputDigital())
		{
			incrementButtons.push_back(kButtonLeftLaserR);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaserOpposite) && IsLaserInputDigital())
		{
			incrementButtons.push_back(kButtonRightLaserR);
		}

		return incrementButtons;
	}

	Array<Button> DecrementButtonsForHorizontalMenu(int32 buttonFlags, FlipArrowKeyDirectionYN flipArrowKeyDirection)
	{
		Array<Button> decrementButtons;

		if (HasFlag(buttonFlags, CursorButtonFlags::kArrow))
		{
			decrementButtons.push_back(flipArrowKeyDirection ? kButtonRight : kButtonLeft);
		}
		
		if (HasFlag(buttonFlags, CursorButtonFlags::kBT))
		{
			decrementButtons.push_back(kButtonBT_A);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBTOpposite))
		{
			decrementButtons.push_back(kButtonBT_C);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kFX))
		{
			decrementButtons.push_back(kButtonFX_L);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaser) && IsLaserInputDigital())
		{
			decrementButtons.push_back(kButtonLeftLaserL);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaserOpposite) && IsLaserInputDigital())
		{
			decrementButtons.push_back(kButtonRightLaserL);
		}

		return decrementButtons;
	}

	Array<Button> IncrementButtonsForVerticalMenu(int32 buttonFlags, FlipArrowKeyDirectionYN flipArrowKeyDirection)
	{
		Array<Button> incrementButtons;

		if (HasFlag(buttonFlags, CursorButtonFlags::kArrow))
		{
			incrementButtons.push_back(flipArrowKeyDirection ? kButtonUp : kButtonDown);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBT))
		{
			incrementButtons.push_back(kButtonBT_B);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBTOpposite))
		{
			incrementButtons.push_back(kButtonBT_D);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kFXOpposite))
		{
			incrementButtons.push_back(kButtonFX_R);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaser) && IsLaserInputDigital())
		{
			incrementButtons.push_back(kButtonRightLaserR);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaserOpposite) && IsLaserInputDigital())
		{
			incrementButtons.push_back(kButtonLeftLaserR);
		}

		return incrementButtons;
	}

	Array<Button> DecrementButtonsForVerticalMenu(int32 buttonFlags, FlipArrowKeyDirectionYN flipArrowKeyDirection)
	{
		Array<Button> decrementButtons;

		if (HasFlag(buttonFlags, CursorButtonFlags::kArrow))
		{
			decrementButtons.push_back(flipArrowKeyDirection ? kButtonDown : kButtonUp);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBT))
		{
			decrementButtons.push_back(kButtonBT_C);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBTOpposite))
		{
			decrementButtons.push_back(kButtonBT_A);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kFXOpposite))
		{
			decrementButtons.push_back(kButtonFX_L);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaser) && IsLaserInputDigital())
		{
			decrementButtons.push_back(kButtonRightLaserL);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaserOpposite) && IsLaserInputDigital())
		{
			decrementButtons.push_back(kButtonLeftLaserL);
		}

		return decrementButtons;
	}

	Array<Button> IncrementButtons(Type type, int32 buttonFlags, FlipArrowKeyDirectionYN flipArrowKeyDirection)
	{
		switch (type)
		{
		case Type::Horizontal:
			return IncrementButtonsForHorizontalMenu(buttonFlags, flipArrowKeyDirection);

		case Type::Vertical:
			return IncrementButtonsForVerticalMenu(buttonFlags, flipArrowKeyDirection);

		default:
			assert(false && "Unknown cursor input type");
			return Array<Button>();
		}
	}

	Array<Button> DecrementButtons(Type type, int32 buttonFlags, FlipArrowKeyDirectionYN flipArrowKeyDirection)
	{
		switch (type)
		{
		case Type::Horizontal:
			return DecrementButtonsForHorizontalMenu(buttonFlags, flipArrowKeyDirection);

		case Type::Vertical:
			return DecrementButtonsForVerticalMenu(buttonFlags, flipArrowKeyDirection);

		default:
			assert(false && "Unknown cursor input type");
			return Array<Button>();
		}
	}
}

CursorInput::CursorInput(const CreateInfo& createInfo)
	: m_buttonDevice(
		IncrementButtons(createInfo.type, createInfo.buttonFlags, createInfo.flipArrowKeyDirection),
		DecrementButtons(createInfo.type, createInfo.buttonFlags, createInfo.flipArrowKeyDirection),
		createInfo.buttonIntervalSec,
		createInfo.buttonIntervalSecFirst,
		createInfo.needStartButtonHoldForNonArrowKey)
	, m_needStartButtonHoldForNonArrowKey(createInfo.needStartButtonHoldForNonArrowKey)
{
	if (HasFlag(createInfo.buttonFlags, CursorButtonFlags::kLaser))
	{
		const int32 laneIdx = createInfo.type == Type::Horizontal ? 0 : 1;
		m_laserDevice = MakeOptional<LaserCursorInputDevice>(laneIdx);
	}

	if (HasFlag(createInfo.buttonFlags, CursorButtonFlags::kLaserOpposite))
	{
		const int32 laneIdx = createInfo.type == Type::Horizontal ? 1 : 0;
		m_laserDeviceOpposite = MakeOptional<LaserCursorInputDevice>(laneIdx);
	}

	if (HasFlag(createInfo.buttonFlags, CursorButtonFlags::kMouseWheel))
	{
		m_mouseWheelDevice = MakeOptional<MouseWheelCursorInputDevice>();
	}
}

void CursorInput::update()
{
	m_buttonDevice.update();

	// アナログ入力時のみLaserCursorInputDeviceを使用(デジタル入力時はButtonCursorInputDeviceで処理済み)
	if (!KeyConfig::IsLaserInputDigital())
	{
		if (m_laserDevice.has_value())
		{
			m_laserDevice->update();
		}
		if (m_laserDeviceOpposite.has_value())
		{
			m_laserDeviceOpposite->update();
		}
	}

	if (m_mouseWheelDevice.has_value())
	{
		m_mouseWheelDevice->update();
	}
}

int32 CursorInput::deltaCursor() const
{
	int32 deltaCursorSum = m_buttonDevice.deltaCursor();

	// アナログ入力時のみLaserCursorInputDeviceを使用
	if (!KeyConfig::IsLaserInputDigital())
	{
		if (!m_needStartButtonHoldForNonArrowKey || KeyConfig::Pressed(kButtonStart))
		{
			if (m_laserDevice.has_value())
			{
				deltaCursorSum += m_laserDevice->deltaCursor();
			}
			if (m_laserDeviceOpposite.has_value())
			{
				deltaCursorSum += m_laserDeviceOpposite->deltaCursor();
			}
		}
	}

	if (m_mouseWheelDevice.has_value())
	{
		deltaCursorSum += m_mouseWheelDevice->deltaCursor();
	}

	return deltaCursorSum;
}
