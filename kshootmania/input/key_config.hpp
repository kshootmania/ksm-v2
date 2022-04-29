#pragma once

namespace KeyConfig
{

	enum ConfigSet : int32
	{
		kKeyboard1 = 0,
		kKeyboard2,
		kGamepad1,
		kGamepad2,

		kConfigSetEnumCount,
	};

	constexpr std::array<StringView, kConfigSetEnumCount> kDefaultConfigValues = {
		U"83,68,75,76,29,28,81,87,79,80,32,13,27,122", // Keyboard 1
		U"72,74,70,71,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1",  // Keyboard 2
		U"-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1",  // Gamepad 1
		U"-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1",  // Gamepad 2
	};

	using Button = int32;

	constexpr Button kUnspecifiedButton = -1;

	// Note: Do not reorder, as it will affect the loading of config.ini.
	enum ConfigurableButton : int32
	{
		kBT_A = 0,
		kBT_B,
		kBT_C,
		kBT_D,
		kFX_L,
		kFX_R,
		kLeftLaserL,
		kLeftLaserR,
		kRightLaserL,
		kRightLaserR,
		kFX_LR,
		kStart,
		kBack,
		kAutoPlay,

		kConfigurableButtonEnumCount,
	};

	enum UnconfigurableButton : int32
	{
		kUp = kConfigurableButtonEnumCount,
		kDown,
		kLeft,
		kRight,

		kBackspace,

		kButtonEnumCount,
	};

	void SetConfigValue(ConfigSet targetConfigSet, StringView configValue);

	bool Pressed(Button button);

	bool Down(Button button);

	bool Up(Button button);

	template <class C>
	bool AnyButtonPressed(const C& buttons)
	{
		for (const auto& button : buttons)
		{
			if (KeyConfig::Pressed(button))
			{
				return true;
			}
		}
		return false;
	}

	template <class C>
	bool AnyButtonDown(const C& buttons)
	{
		for (const auto& button : buttons)
		{
			if (KeyConfig::Down(button))
			{
				return true;
			}
		}
		return false;
	}

	template <class C>
	bool AnyButtonUp(const C& buttons)
	{
		for (const auto& button : buttons)
		{
			if (KeyConfig::Up(button))
			{
				return true;
			}
		}
		return false;
	}
}
