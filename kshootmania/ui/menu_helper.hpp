#pragma once
#include "linear_menu.hpp"

namespace MenuHelper
{
	namespace ButtonFlags
	{
		enum ButtonFlags : int32
		{
			kArrow = 1 << 0,
			kBT = 1 << 1,
			kBTOpposite = 1 << 2,
			kFX = 1 << 3,
			kFXOpposite = 1 << 4,
			kLaser = 1 << 5,
			kLaserOpposite = 1 << 6,

			kArrowOrBT = kArrow | kBT,
			kArrowOrBTAll = kArrow | kBT | kBTOpposite,
			kArrowOrFX = kArrow | kFX,
			kArrowOrLaser = kArrow | kLaser,
			kArrowOrLaserAll = kArrow | kLaser | kLaserOpposite,
			kArrowOrBTOrLaser = kArrow | kBT | kLaser,
			kArrowOrBTOrFXOrLaser = kArrow | kBT | kFX | kLaser,
		};
	}

	template <typename T>
	LinearMenu MakeHorizontalMenuWithMinMax(
		T cursorMin,
		T cursorMax,
		int32 buttonFlags = kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		int32 cursorStep = 1)
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

		return LinearMenu(
			cursorMin,
			cursorMax,
			incrementButtons,
			decrementButtons,
			cyclic,
			intervalSec,
			cursorStep);
	}

	template <typename T>
	LinearMenu MakeHorizontalMenu(
		T enumCount,
		int32 buttonFlags = kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		int32 cursorStep = 1)
	{
		return MakeHorizontalMenuWithMinMax(
			static_cast<T>(0),
			static_cast<T>(enumCount - 1),
			buttonFlags,
			cyclic,
			intervalSec,
			cursorStep);
	}

	template <typename T>
	LinearMenu MakeVerticalMenuWithMinMax(
		T cursorMin,
		T cursorMax,
		int32 buttonFlags = kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		int32 cursorStep = 1)
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

		return LinearMenu(
			cursorMin,
			cursorMax,
			incrementButtons,
			decrementButtons,
			cyclic,
			intervalSec,
			cursorStep);
	}

	template <typename T>
	LinearMenu MakeVerticalMenu(
		T enumCount,
		int32 buttonFlags = kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		int32 cursorStep = 1)
	{
		return MakeVerticalMenuWithMinMax(
			static_cast<T>(0),
			static_cast<T>(enumCount - 1),
			buttonFlags,
			cyclic,
			intervalSec,
			cursorStep);
	}
}
