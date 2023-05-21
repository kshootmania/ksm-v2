#include "key_config.hpp"

namespace
{
	// キーコンフィグの設定画面や保存時などに配列サイズが固定のほうが都合が良いのでs3d::InputGroupは不使用
	using ConfigSet = std::array<Input, KeyConfig::kButtonEnumCount>;
	std::array<ConfigSet, KeyConfig::kConfigSetEnumCount> s_configSetArray;

	constexpr std::array<InputDeviceType, KeyConfig::kConfigSetEnumCount> kConfigSetDeviceTypes = {
		InputDeviceType::Keyboard,
		InputDeviceType::Keyboard,
		InputDeviceType::Gamepad,
		InputDeviceType::Gamepad,
	};

	constexpr std::array<StringView, KeyConfig::kConfigSetEnumCount> kConfigSetNames = {
		U"Keyboard 1",
		U"Keyboard 2",
		U"Gamepad 1",
		U"Gamepad 2",
	};
}

void KeyConfig::SetConfigValue(ConfigSet targetConfigSet, StringView configValue)
{
	Array<String> values = String(configValue).split(U',');

	if (targetConfigSet < 0 || kConfigSetEnumCount <= targetConfigSet)
	{
		Print << U"Warning: Invalid key config target '{}'!"_fmt(targetConfigSet);
		return;
	}

	if (values.size() != kConfigurableButtonEnumCount)
	{
		Print << U"Warning: Key configuration ({}) is ignored because value count does not match! (expected:{}, actual:{})"_fmt(kConfigSetNames[targetConfigSet], kConfigurableButtonEnumCount, values.size());
		values = String(kDefaultConfigValues[targetConfigSet]).split(U',');
	}

	Array<int32> intValues;
	try
	{
		intValues = values.map([](const String& str) { return Parse<int32>(str); });
	}
	catch (const ParseError&)
	{
		Print << U"Warning: Key configuration ({}) is ignored due to parse error!"_fmt(kConfigSetNames[targetConfigSet]);
		try
		{
			intValues = String(kDefaultConfigValues[targetConfigSet]).split(U',').map([](const String& str) { return Parse<int32>(str); });
		}
		catch (const ParseError&)
		{
			throw Error(U"KeyConfig::SetConfigValue(): Could not parse KeyConfig::kDefaultConfigValues!");
		}
	}

	for (int32 i = 0; i < kConfigurableButtonEnumCount; ++i)
	{
		if (0 <= intValues[i] && intValues[i] < 0x100)
		{
			s_configSetArray[targetConfigSet][i] = Input(kConfigSetDeviceTypes[targetConfigSet], static_cast<uint8>(intValues[i]));
		}
		else
		{
			s_configSetArray[targetConfigSet][i] = Input();
		}
	}

	// Keyboard 1の場合、ユーザーによって変更できない固定のキーコンフィグがあるので上書き
	if (targetConfigSet == kKeyboard1)
	{
		s_configSetArray[kKeyboard1][kStart] = KeyEnter;
		s_configSetArray[kKeyboard1][kBack] = KeyEscape;
		s_configSetArray[kKeyboard1][kAutoPlay] = KeyF11;
		s_configSetArray[kKeyboard1][kUp] = KeyUp;
		s_configSetArray[kKeyboard1][kDown] = KeyDown;
		s_configSetArray[kKeyboard1][kLeft] = KeyLeft;
		s_configSetArray[kKeyboard1][kRight] = KeyRight;
		s_configSetArray[kKeyboard1][kBackspace] = KeyBackspace;
	}
}

bool KeyConfig::Pressed(Button button)
{
	if (button == KeyConfig::kUnspecifiedButton)
	{
		return false;
	}

	for (const auto& configSet : s_configSetArray)
	{
		if (configSet[button].pressed())
		{
			return true;
		}
	}

	// FXの場合はLR両押しキーの状態も反映
	if (button == kFX_L || button == kFX_R)
	{
		for (const auto& configSet : s_configSetArray)
		{
			if (configSet[kFX_LR].pressed())
			{
				return true;
			}
		}
	}
	
	return false;
}

Optional<KeyConfig::Button> KeyConfig::LastPressedLaserButton(Button button1, Button button2)
{
	assert(button1 == kLeftLaserL || button1 == kLeftLaserR || button1 == kRightLaserL || button1 == kRightLaserR);
	assert(button2 == kLeftLaserL || button2 == kLeftLaserR || button2 == kRightLaserL || button2 == kRightLaserR);

	Optional<Button> lastButton = none;
	Duration minDuration = Duration::zero();
	for (const auto& configSet : s_configSetArray)
	{
		for (Button button : { button1, button2 })
		{
			if (configSet[button].pressed())
			{
				const Duration duration = configSet[button].pressedDuration();
				if (!lastButton.has_value() || duration < minDuration)
				{
					minDuration = duration;
					lastButton = button;
				}
			}
		}
	}

	return lastButton;
}

bool KeyConfig::Down(Button button)
{
	if (button == KeyConfig::kUnspecifiedButton)
	{
		return false;
	}

	for (const auto& configSet : s_configSetArray)
	{
		if (configSet[button].down())
		{
			return true;
		}
	}

	// FXの場合はLR両押しキーの状態も反映
	if (button == kFX_L || button == kFX_R)
	{
		for (const auto& configSet : s_configSetArray)
		{
			if (configSet[kFX_LR].down())
			{
				return true;
			}
		}
	}

	return false;
}

bool KeyConfig::Up(Button button)
{
	if (button == KeyConfig::kUnspecifiedButton)
	{
		return false;
	}

	// あるボタンを離したと判定するには、そのボタンに対応する全キーを離している必要がある
	if (Pressed(button))
	{
		return false;
	}

	for (const auto& configSet : s_configSetArray)
	{
		if (configSet[button].up())
		{
			return true;
		}
	}
	return false;
}
