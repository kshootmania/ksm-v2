#include "cursor_input.hpp"
#include "button_cursor_input_device.hpp"

namespace
{
	using Type = CursorInput::Type;

	bool HasFlag(int32 buttonFlags, CursorButtonFlags::CursorButtonFlags flag)
	{
		return (buttonFlags & static_cast<int32>(flag)) != 0;
	}

	Array<KeyConfig::Button> IncrementButtonsForHorizontalMenu(int32 buttonFlags, FlipArrowKeyDirectionYN flipArrowKeyDirection)
	{
		Array<KeyConfig::Button> incrementButtons;

		if (HasFlag(buttonFlags, CursorButtonFlags::kArrow))
		{
			incrementButtons.push_back(flipArrowKeyDirection ? KeyConfig::kLeft : KeyConfig::kRight);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBT))
		{
			incrementButtons.push_back(KeyConfig::kBT_D);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBTOpposite))
		{
			incrementButtons.push_back(KeyConfig::kBT_B);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kFX))
		{
			incrementButtons.push_back(KeyConfig::kFX_R);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaser))
		{
			incrementButtons.push_back(KeyConfig::kLeftLaserR);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaserOpposite))
		{
			incrementButtons.push_back(KeyConfig::kRightLaserR);
		}

		return incrementButtons;
	}

	Array<KeyConfig::Button> DecrementButtonsForHorizontalMenu(int32 buttonFlags, FlipArrowKeyDirectionYN flipArrowKeyDirection)
	{
		Array<KeyConfig::Button> decrementButtons;

		if (HasFlag(buttonFlags, CursorButtonFlags::kArrow))
		{
			decrementButtons.push_back(flipArrowKeyDirection ? KeyConfig::kRight : KeyConfig::kLeft);
		}
		
		if (HasFlag(buttonFlags, CursorButtonFlags::kBT))
		{
			decrementButtons.push_back(KeyConfig::kBT_A);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBTOpposite))
		{
			decrementButtons.push_back(KeyConfig::kBT_C);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kFX))
		{
			decrementButtons.push_back(KeyConfig::kFX_L);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaser))
		{
			decrementButtons.push_back(KeyConfig::kLeftLaserL);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaserOpposite))
		{
			decrementButtons.push_back(KeyConfig::kRightLaserL);
		}

		return decrementButtons;
	}

	Array<KeyConfig::Button> IncrementButtonsForVerticalMenu(int32 buttonFlags, FlipArrowKeyDirectionYN flipArrowKeyDirection)
	{
		Array<KeyConfig::Button> incrementButtons;

		if (HasFlag(buttonFlags, CursorButtonFlags::kArrow))
		{
			incrementButtons.push_back(flipArrowKeyDirection ? KeyConfig::kUp : KeyConfig::kDown);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBT))
		{
			incrementButtons.push_back(KeyConfig::kBT_B);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBTOpposite))
		{
			incrementButtons.push_back(KeyConfig::kBT_D);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kFXOpposite))
		{
			incrementButtons.push_back(KeyConfig::kFX_R);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaser))
		{
			incrementButtons.push_back(KeyConfig::kRightLaserR);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaserOpposite))
		{
			incrementButtons.push_back(KeyConfig::kLeftLaserR);
		}

		return incrementButtons;
	}

	Array<KeyConfig::Button> DecrementButtonsForVerticalMenu(int32 buttonFlags, FlipArrowKeyDirectionYN flipArrowKeyDirection)
	{
		Array<KeyConfig::Button> decrementButtons;

		if (HasFlag(buttonFlags, CursorButtonFlags::kArrow))
		{
			decrementButtons.push_back(flipArrowKeyDirection ? KeyConfig::kDown : KeyConfig::kUp);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBT))
		{
			decrementButtons.push_back(KeyConfig::kBT_C);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kBTOpposite))
		{
			decrementButtons.push_back(KeyConfig::kBT_A);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kFXOpposite))
		{
			decrementButtons.push_back(KeyConfig::kFX_L);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaser))
		{
			decrementButtons.push_back(KeyConfig::kRightLaserL);
		}

		if (HasFlag(buttonFlags, CursorButtonFlags::kLaserOpposite))
		{
			decrementButtons.push_back(KeyConfig::kLeftLaserL);
		}

		return decrementButtons;
	}

	Array<KeyConfig::Button> IncrementButtons(Type type, int32 buttonFlags, FlipArrowKeyDirectionYN flipArrowKeyDirection)
	{
		switch (type)
		{
		case Type::Horizontal:
			return IncrementButtonsForHorizontalMenu(buttonFlags, flipArrowKeyDirection);

		case Type::Vertical:
			return IncrementButtonsForVerticalMenu(buttonFlags, flipArrowKeyDirection);

		default:
			assert(false && "Unknown cursor input type");
			return Array<KeyConfig::Button>();
		}
	}

	Array<KeyConfig::Button> DecrementButtons(Type type, int32 buttonFlags, FlipArrowKeyDirectionYN flipArrowKeyDirection)
	{
		switch (type)
		{
		case Type::Horizontal:
			return DecrementButtonsForHorizontalMenu(buttonFlags, flipArrowKeyDirection);

		case Type::Vertical:
			return DecrementButtonsForVerticalMenu(buttonFlags, flipArrowKeyDirection);

		default:
			assert(false && "Unknown cursor input type");
			return Array<KeyConfig::Button>();
		}
	}
}

CursorInput::CursorInput(const CreateInfo& createInfo)
	: m_buttonDevice(
		IncrementButtons(createInfo.type, createInfo.buttonFlags, createInfo.flipArrowKeyDirection),
		DecrementButtons(createInfo.type, createInfo.buttonFlags, createInfo.flipArrowKeyDirection),
		createInfo.buttonIntervalSec,
		createInfo.buttonIntervalSecFirst,
		createInfo.startRequiredForBTFXLaser)
{
}

void CursorInput::update()
{
	m_buttonDevice.update();
}

int32 CursorInput::deltaCursor() const
{
	int32 deltaCursorSum = 0;
	deltaCursorSum += m_buttonDevice.deltaCursor();
	return deltaCursorSum;
}
