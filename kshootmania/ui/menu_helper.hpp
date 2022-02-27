#pragma once
#include "linear_menu.hpp"

namespace MenuHelper
{
	namespace ButtonFlags
	{
		enum ButtonFlags : int32
		{
			kArrow         = 1 << 0,
			kBT            = 1 << 1,
			kBTOpposite    = 1 << 2,
			kFX            = 1 << 3,
			kFXOpposite    = 1 << 4,
			kLaser         = 1 << 5,
			kLaserOpposite = 1 << 6,
		};
	}

	template <typename T>
	LinearMenu<T> MakeHorizontalMenu(
		T enumCount,
		int32 buttonFlags = kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0)
	{
		Array<KeyConfig::Button> incrementButtons, decrementButtons;

		if (buttonFlags & ButtonFlags::kArrow)
		{
			decrementButtons.push_back(KeyConfig::kLeft);
			incrementButtons.push_back(KeyConfig::kRight);
		}

		if (buttonFlags & ButtonFlags::kBT)
		{
			decrementButtons.push_back(KeyConfig::kBT_A);
			incrementButtons.push_back(KeyConfig::kBT_D);
		}

		if (buttonFlags & ButtonFlags::kBTOpposite)
		{
			decrementButtons.push_back(KeyConfig::kBT_C);
			incrementButtons.push_back(KeyConfig::kBT_B);
		}

		if (buttonFlags & ButtonFlags::kFX)
		{
			decrementButtons.push_back(KeyConfig::kFX_L);
			incrementButtons.push_back(KeyConfig::kFX_R);
		}

		if (buttonFlags & ButtonFlags::kLaser)
		{
			decrementButtons.push_back(KeyConfig::kRightLaserL);
			incrementButtons.push_back(KeyConfig::kRightLaserR);
		}

		if (buttonFlags & ButtonFlags::kLaserOpposite)
		{
			decrementButtons.push_back(KeyConfig::kLeftLaserL);
			incrementButtons.push_back(KeyConfig::kLeftLaserR);
		}

		return LinearMenu<T>(
			static_cast<T>(0),
			static_cast<T>(static_cast<int32>(enumCount) - 1),
			incrementButtons,
			decrementButtons,
			cyclic,
			intervalSec);
	}

	template <typename T>
	LinearMenu<T> MakeVerticalMenu(
		T enumCount,
		int32 buttonFlags = kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0)
	{
		Array<KeyConfig::Button> incrementButtons, decrementButtons;

		if (buttonFlags & ButtonFlags::kArrow)
		{
			decrementButtons.push_back(KeyConfig::kUp);
			incrementButtons.push_back(KeyConfig::kDown);
		}

		if (buttonFlags & ButtonFlags::kBT)
		{
			decrementButtons.push_back(KeyConfig::kBT_C);
			incrementButtons.push_back(KeyConfig::kBT_B);
		}

		if (buttonFlags & ButtonFlags::kBTOpposite)
		{
			decrementButtons.push_back(KeyConfig::kBT_A);
			incrementButtons.push_back(KeyConfig::kBT_D);
		}

		if (buttonFlags & ButtonFlags::kFXOpposite)
		{
			decrementButtons.push_back(KeyConfig::kFX_L);
			incrementButtons.push_back(KeyConfig::kFX_R);
		}

		if (buttonFlags & ButtonFlags::kLaser)
		{
			decrementButtons.push_back(KeyConfig::kLeftLaserL);
			incrementButtons.push_back(KeyConfig::kLeftLaserR);
		}

		if (buttonFlags & ButtonFlags::kLaserOpposite)
		{
			decrementButtons.push_back(KeyConfig::kRightLaserL);
			incrementButtons.push_back(KeyConfig::kRightLaserR);
		}

		return LinearMenu<T>(
			static_cast<T>(0),
			static_cast<T>(static_cast<int32>(enumCount) - 1),
			incrementButtons,
			decrementButtons,
			cyclic,
			intervalSec);
	}
}
