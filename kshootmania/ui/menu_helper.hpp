#pragma once
#include "linear_menu.hpp"
#include "array_with_linear_menu.hpp"

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

	Array<KeyConfig::Button> IncrementButtonsForHorizontalMenu(int32 buttonFlags);

	Array<KeyConfig::Button> DecrementButtonsForHorizontalMenu(int32 buttonFlags);

	template <typename T>
	LinearMenu MakeHorizontalMenuWithMinMax(
		T cursorMin,
		T cursorMax,
		int32 buttonFlags = ButtonFlags::kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		int32 defaultCursor = std::numeric_limits<int32>::min(),
		int32 cursorStep = 1)
	{
		return LinearMenu(
			cursorMin,
			cursorMax,
			IncrementButtonsForHorizontalMenu(buttonFlags),
			DecrementButtonsForHorizontalMenu(buttonFlags),
			cyclic,
			intervalSec,
			defaultCursor,
			cursorStep);
	}

	template <typename T>
	LinearMenu MakeHorizontalMenu(
		T enumCount,
		int32 buttonFlags = ButtonFlags::kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		int32 defaultCursor = 0,
		int32 cursorStep = 1)
	{
		return MakeHorizontalMenuWithMinMax(
			0,
			std::max(static_cast<int32>(enumCount) - 1, 0),
			buttonFlags,
			cyclic,
			intervalSec,
			defaultCursor,
			cursorStep);
	}

	template <typename T>
	ArrayWithLinearMenu<T> MakeArrayBindedHorizontalMenu(
		int32 buttonFlags = ButtonFlags::kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		int32 defaultCursor = 0,
		int32 cursorStep = 1)
	{
		return ArrayWithLinearMenu<T>(
			IncrementButtonsForHorizontalMenu(buttonFlags),
			DecrementButtonsForHorizontalMenu(buttonFlags),
			cyclic,
			intervalSec,
			defaultCursor,
			cursorStep);
	}

	template <typename T>
	ArrayWithLinearMenu<T> MakeArrayBindedHorizontalMenu(
		const Array<T>& array,
		int32 buttonFlags = ButtonFlags::kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		int32 defaultCursor = 0,
		int32 cursorStep = 1)
	{
		return ArrayWithLinearMenu<T>(
			array,
			IncrementButtonsForHorizontalMenu(buttonFlags),
			DecrementButtonsForHorizontalMenu(buttonFlags),
			cyclic,
			intervalSec,
			defaultCursor,
			cursorStep);
	}

	Array<KeyConfig::Button> IncrementButtonsForVerticalMenu(int32 buttonFlags);

	Array<KeyConfig::Button> DecrementButtonsForVerticalMenu(int32 buttonFlags);

	template <typename T>
	LinearMenu MakeVerticalMenuWithMinMax(
		T cursorMin,
		T cursorMax,
		int32 buttonFlags = ButtonFlags::kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		int32 defaultCursor = std::numeric_limits<int32>::min(),
		int32 cursorStep = 1)
	{
		return LinearMenu(
			cursorMin,
			cursorMax,
			IncrementButtonsForVerticalMenu(buttonFlags),
			DecrementButtonsForVerticalMenu(buttonFlags),
			cyclic,
			intervalSec,
			defaultCursor,
			cursorStep);
	}

	template <typename T>
	LinearMenu MakeVerticalMenu(
		T enumCount,
		int32 buttonFlags = ButtonFlags::kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		int32 defaultCursor = 0,
		int32 cursorStep = 1)
	{
		return MakeVerticalMenuWithMinMax(
			0,
			std::max(static_cast<int32>(enumCount) - 1, 0),
			buttonFlags,
			cyclic,
			intervalSec,
			defaultCursor,
			cursorStep);
	}

	template <typename T>
	ArrayWithLinearMenu<T> MakeArrayBindedVerticalMenu(
		int32 buttonFlags = ButtonFlags::kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		int32 defaultCursor = 0,
		int32 cursorStep = 1)
	{
		return ArrayWithLinearMenu<T>(
			IncrementButtonsForVerticalMenu(buttonFlags),
			DecrementButtonsForVerticalMenu(buttonFlags),
			cyclic,
			intervalSec,
			defaultCursor,
			cursorStep);
	}

	template <typename T>
	ArrayWithLinearMenu<T> MakeArrayBindedVerticalMenu(
		const Array<T>& array,
		int32 buttonFlags = ButtonFlags::kArrow,
		IsCyclicMenu cyclic = IsCyclicMenu::No,
		double intervalSec = 0.0,
		int32 defaultCursor = 0,
		int32 cursorStep = 1)
	{
		return ArrayWithLinearMenu<T>(
			array,
			IncrementButtonsForVerticalMenu(buttonFlags),
			DecrementButtonsForVerticalMenu(buttonFlags),
			cyclic,
			intervalSec,
			defaultCursor,
			cursorStep);
	}
}
