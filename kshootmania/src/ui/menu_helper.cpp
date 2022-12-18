#include "menu_helper.hpp"

Array<KeyConfig::Button> MenuHelper::IncrementButtonsForHorizontalMenu(int32 buttonFlags)
{
	Array<KeyConfig::Button> incrementButtons;

	if (buttonFlags & ButtonFlags::kArrow)
	{
		incrementButtons.push_back(KeyConfig::kRight);
	}

	if (buttonFlags & ButtonFlags::kBT)
	{
		incrementButtons.push_back(KeyConfig::kBT_D);
	}

	if (buttonFlags & ButtonFlags::kBTOpposite)
	{
		incrementButtons.push_back(KeyConfig::kBT_B);
	}

	if (buttonFlags & ButtonFlags::kFX)
	{
		incrementButtons.push_back(KeyConfig::kFX_R);
	}

	if (buttonFlags & ButtonFlags::kLaser)
	{
		incrementButtons.push_back(KeyConfig::kLeftLaserR);
	}

	if (buttonFlags & ButtonFlags::kLaserOpposite)
	{
		incrementButtons.push_back(KeyConfig::kRightLaserR);
	}

	return incrementButtons;
}

Array<KeyConfig::Button> MenuHelper::DecrementButtonsForHorizontalMenu(int32 buttonFlags)
{
	Array<KeyConfig::Button> decrementButtons;

	if (buttonFlags & ButtonFlags::kArrow)
	{
		decrementButtons.push_back(KeyConfig::kLeft);
	}

	if (buttonFlags & ButtonFlags::kBT)
	{
		decrementButtons.push_back(KeyConfig::kBT_A);
	}

	if (buttonFlags & ButtonFlags::kBTOpposite)
	{
		decrementButtons.push_back(KeyConfig::kBT_C);
	}

	if (buttonFlags & ButtonFlags::kFX)
	{
		decrementButtons.push_back(KeyConfig::kFX_L);
	}

	if (buttonFlags & ButtonFlags::kLaser)
	{
		decrementButtons.push_back(KeyConfig::kLeftLaserL);
	}

	if (buttonFlags & ButtonFlags::kLaserOpposite)
	{
		decrementButtons.push_back(KeyConfig::kRightLaserL);
	}

	return decrementButtons;
}

Array<KeyConfig::Button> MenuHelper::IncrementButtonsForVerticalMenu(int32 buttonFlags)
{
	Array<KeyConfig::Button> incrementButtons;

	if (buttonFlags & ButtonFlags::kArrow)
	{
		incrementButtons.push_back(KeyConfig::kDown);
	}

	if (buttonFlags & ButtonFlags::kBT)
	{
		incrementButtons.push_back(KeyConfig::kBT_B);
	}

	if (buttonFlags & ButtonFlags::kBTOpposite)
	{
		incrementButtons.push_back(KeyConfig::kBT_D);
	}

	if (buttonFlags & ButtonFlags::kFXOpposite)
	{
		incrementButtons.push_back(KeyConfig::kFX_R);
	}

	if (buttonFlags & ButtonFlags::kLaser)
	{
		incrementButtons.push_back(KeyConfig::kRightLaserR);
	}

	if (buttonFlags & ButtonFlags::kLaserOpposite)
	{
		incrementButtons.push_back(KeyConfig::kLeftLaserR);
	}

	return incrementButtons;
}

Array<KeyConfig::Button> MenuHelper::DecrementButtonsForVerticalMenu(int32 buttonFlags)
{
	Array<KeyConfig::Button> decrementButtons;

	if (buttonFlags & ButtonFlags::kArrow)
	{
		decrementButtons.push_back(KeyConfig::kUp);
	}

	if (buttonFlags & ButtonFlags::kBT)
	{
		decrementButtons.push_back(KeyConfig::kBT_C);
	}

	if (buttonFlags & ButtonFlags::kBTOpposite)
	{
		decrementButtons.push_back(KeyConfig::kBT_A);
	}

	if (buttonFlags & ButtonFlags::kFXOpposite)
	{
		decrementButtons.push_back(KeyConfig::kFX_L);
	}

	if (buttonFlags & ButtonFlags::kLaser)
	{
		decrementButtons.push_back(KeyConfig::kRightLaserL);
	}

	if (buttonFlags & ButtonFlags::kLaserOpposite)
	{
		decrementButtons.push_back(KeyConfig::kLeftLaserL);
	}

	return decrementButtons;
}
