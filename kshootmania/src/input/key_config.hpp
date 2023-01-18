#pragma once

using StartRequiredForBTFXLaserYN = YesNo<struct StartRequiredForBTFXLaserYN_tag>;

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

	constexpr bool IsButtonBTFXLaser(Button button)
	{
		switch (button)
		{
		case kBT_A:
		case kBT_B:
		case kBT_C:
		case kBT_D:
		case kFX_L:
		case kFX_R:
		case kLeftLaserL:
		case kLeftLaserR:
		case kRightLaserL:
		case kRightLaserR:
			return true;

		default:
			return false;
		}
	}

	void SetConfigValue(ConfigSet targetConfigSet, StringView configValue);

	void Update();

	bool Pressed(Button button);

	Optional<KeyConfig::Button> LastPressed(Button button1, Button button2);

	bool Down(Button button, int32* pDiffMillisecond = nullptr);

	bool Up(Button button);

	template <class C>
	bool AnyButtonPressed(const C& buttons, StartRequiredForBTFXLaserYN startRequiredForBTFXLaser = StartRequiredForBTFXLaserYN::No)
	{
		const bool btFXLaserAccepted = !startRequiredForBTFXLaser || KeyConfig::Pressed(kStart);
		for (const auto& button : buttons)
		{
			const bool accepted = btFXLaserAccepted || !IsButtonBTFXLaser(button);
			if (accepted && KeyConfig::Pressed(button))
			{
				return true;
			}
		}
		return false;
	}

	template <class C>
	bool AnyButtonDown(const C& buttons, StartRequiredForBTFXLaserYN startRequiredForBTFXLaser = StartRequiredForBTFXLaserYN::No)
	{
		const bool btFXLaserAccepted = !startRequiredForBTFXLaser || KeyConfig::Pressed(kStart); // Startボタン判定側は押しっぱなしかの判定なのでPressedで正しい
		for (const auto& button : buttons)
		{
			const bool accepted = btFXLaserAccepted || !IsButtonBTFXLaser(button);
			if (accepted && KeyConfig::Down(button))
			{
				return true;
			}
		}
		return false;
	}
}
