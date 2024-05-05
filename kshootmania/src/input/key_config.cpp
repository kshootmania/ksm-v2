#include "key_config.hpp"

namespace
{
	// キーコンフィグの設定画面や保存時などに配列サイズが固定のほうが都合が良いのでs3d::InputGroupは不使用
	using ConfigSetArray = std::array<Input, KeyConfig::kButtonEnumCount>;
	std::array<ConfigSetArray, KeyConfig::kConfigSetEnumCount> s_configSetArray;

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

	StringView ConfigSetToConfigIniKey(KeyConfig::ConfigSet configSet)
	{
		switch (configSet)
		{
		case KeyConfig::kKeyboard1:
			return ConfigIni::Key::kKeyConfigKeyboard1;
		case KeyConfig::kKeyboard2:
			return ConfigIni::Key::kKeyConfigKeyboard2;
		case KeyConfig::kGamepad1:
			return ConfigIni::Key::kKeyConfigGamepad1;
		case KeyConfig::kGamepad2:
			return ConfigIni::Key::kKeyConfigGamepad2;
		default:
			throw Error(U"ConfigSetToConfigIniKey(): Invalid configSet");
		}
	}

	void RevertUnconfigurableKeyConfigs()
	{
		// Keyboard 1の場合、ユーザーによって変更できない固定のキーコンフィグがあるので上書き
		s_configSetArray[KeyConfig::kKeyboard1][KeyConfig::kStart] = KeyEnter;
		s_configSetArray[KeyConfig::kKeyboard1][KeyConfig::kBack] = KeyEscape;
		s_configSetArray[KeyConfig::kKeyboard1][KeyConfig::kAutoPlay] = KeyF11;
		s_configSetArray[KeyConfig::kKeyboard1][KeyConfig::kUp] = KeyUp;
		s_configSetArray[KeyConfig::kKeyboard1][KeyConfig::kDown] = KeyDown;
		s_configSetArray[KeyConfig::kKeyboard1][KeyConfig::kLeft] = KeyLeft;
		s_configSetArray[KeyConfig::kKeyboard1][KeyConfig::kRight] = KeyRight;
		s_configSetArray[KeyConfig::kKeyboard1][KeyConfig::kBackspace] = KeyBackspace;
	}
}

void KeyConfig::SetConfigValueByCommaSeparated(ConfigSet targetConfigSet, StringView configValue)
{
	Array<String> values = String(configValue).split(U',');

	if (targetConfigSet < 0 || kConfigSetEnumCount <= targetConfigSet)
	{
		Print << U"Warning: Invalid key config target '{}'!"_fmt(std::to_underlying(targetConfigSet));
		return;
	}

	if (values.size() != kConfigurableButtonEnumCount)
	{
		Print << U"Warning: Key configuration ({}) is ignored because value count does not match! (expected:{}, actual:{})"_fmt(kConfigSetNames[targetConfigSet], std::to_underlying(kConfigurableButtonEnumCount), values.size());
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

	RevertUnconfigurableKeyConfigs();
}

void KeyConfig::SetConfigValue(ConfigSet targetConfigSet, ConfigurableButton button, const Input& input)
{
	if (targetConfigSet < 0 || kConfigSetEnumCount <= targetConfigSet)
	{
		throw Error(U"Warning: Invalid key config target '{}'!"_fmt(std::to_underlying(targetConfigSet)));
	}

	if (button < 0 || kConfigurableButtonEnumCount <= button)
	{
		throw Error(U"Warning: Invalid key config button '{}'!"_fmt(std::to_underlying(button)));
	}

	s_configSetArray[targetConfigSet][button] = input;

	RevertUnconfigurableKeyConfigs();
}

const Input& KeyConfig::GetConfigValue(ConfigSet targetConfigSet, ConfigurableButton button)
{
	if (targetConfigSet < 0 || kConfigSetEnumCount <= targetConfigSet)
	{
		throw Error(U"Warning: Invalid key config target '{}'!"_fmt(std::to_underlying(targetConfigSet)));
	}

	if (button < 0 || kConfigurableButtonEnumCount <= button)
	{
		throw Error(U"Warning: Invalid key config button '{}'!"_fmt(std::to_underlying(button)));
	}

	return s_configSetArray[targetConfigSet][button];
}

void KeyConfig::SaveToConfigIni()
{
	for (int32 configSetIdx = 0; configSetIdx < kConfigSetEnumCount; ++configSetIdx)
	{
		// キーコード一覧をカンマ区切りの文字列に変換
		String configValue;
		for (int32 i = 0; i < kConfigurableButtonEnumCount; ++i)
		{
			int32 code;
			if (s_configSetArray[configSetIdx][i].deviceType() == InputDeviceType::Undefined)
			{
				code = -1;
			}
			else
			{
				code = static_cast<int32>(s_configSetArray[configSetIdx][i].code());
			}
			configValue += Format(code);
			configValue += U",";
		}
		configValue.pop_back();

		// 保存
		const auto configSet = static_cast<ConfigSet>(configSetIdx);
		const StringView configIniKey = ConfigSetToConfigIniKey(configSet);
		ConfigIni::SetString(configIniKey, configValue);
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

void KeyConfig::ClearInput(Button button)
{
	if (button == KeyConfig::kUnspecifiedButton)
	{
		return;
	}

	for (auto& configSet : s_configSetArray)
	{
		configSet[button].clearInput();
	}

	// FXの場合はLR両押しキーの状態もクリア
	if (button == kFX_L || button == kFX_R)
	{
		for (auto& configSet : s_configSetArray)
		{
			configSet[kFX_LR].clearInput();
		}
	}
}

Co::Task<void> KeyConfig::WaitForDown(Button button)
{
	co_await Co::WaitUntil([button]() { return Down(button); });

	// 即座に次シーンに遷移した場合に多重に反応しないよう、入力をクリアする必要がある
	ClearInput(button);
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
