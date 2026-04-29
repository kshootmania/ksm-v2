#include "KeyConfig.hpp"
#include "LaserInput/KeyboardLaserInput.hpp"
#include "LaserInput/AnalogStickXYLaserInput.hpp"
#include "LaserInput/SliderLaserInput.hpp"
#include "LaserInput/MouseXYLaserInput.hpp"
#include "Ini/ConfigIni.hpp"

namespace
{
	using namespace KeyConfig;

	// キーコンフィグの設定画面や保存時などに配列サイズが固定のほうが都合が良いのでs3d::InputGroupは不使用
	using ConfigSetArray = std::array<Input, kButtonEnumCount>;
	std::array<ConfigSetArray, kConfigSetEnumCount> s_configSetArray;

	// レーザー入力方式のインスタンス
	std::array<std::unique_ptr<ILaserInputMethod>, kson::kNumLaserLanesSZ> s_laserInputMethods;

	// IMEで編集中のテキストがあるかどうかに前フレームを加味するための変数
	bool s_currentEditingTextExists = false;
	bool s_prevEditingTextExists = false;
	int32 s_lastEditingTextUpdateFrame = -1;

	[[nodiscard]]
	bool ShouldIgnoreKeyboardInput()
	{
		if (!noco::IsEditingTextBox())
		{
			s_currentEditingTextExists = false;
			s_prevEditingTextExists = false;
			return false;
		}

		const int32 currentFrame = Scene::FrameCount();
		if (s_lastEditingTextUpdateFrame != currentFrame)
		{
			s_lastEditingTextUpdateFrame = currentFrame;
			s_prevEditingTextExists = s_currentEditingTextExists;
			s_currentEditingTextExists = !TextInput::GetEditingText().empty();
		}

		// IMEで編集中のテキストがある場合はキーボード入力はボタン入力としては無視
		// (Enter確定時はGetEditingTextが空になるため前フレームも確認)
		return s_currentEditingTextExists || s_prevEditingTextExists;
	}

#ifdef __APPLE__
	struct PlatformKeyState
	{
		bool previousPressed = false;
		bool currentPressed = false;

		void update(bool pressed)
		{
			previousPressed = currentPressed;
			currentPressed = pressed;
		}

		bool pressed() const { return currentPressed; }
		bool down() const { return currentPressed && !previousPressed; }
		bool up() const { return !currentPressed && previousPressed; }
	};

	std::array<PlatformKeyState, kPlatformKeys.size()> s_platformKeyStates;
#endif

	constexpr std::array<InputDeviceType, kConfigSetEnumCount> kConfigSetDeviceTypes = {
		InputDeviceType::Keyboard,
		InputDeviceType::Keyboard,
		InputDeviceType::Gamepad,
		InputDeviceType::Gamepad,
	};

	constexpr std::array<StringView, kConfigSetEnumCount> kConfigSetNames = {
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

	enum UseNumpadAsArrowKeys : int32
	{
		kOff = 0,
		kOnKeyboard,
		kOnController,
	};

	UseNumpadAsArrowKeys GetUseNumpadAsArrowKeysMode()
	{
		const int32 mode = ConfigIni::GetInt(ConfigIni::Key::kUseNumpadAsArrowKeys, 0);
		if (mode < 0 || mode > kOnController)
		{
			return kOff;
		}
		return static_cast<UseNumpadAsArrowKeys>(mode);
	}

	bool IsNumpadArrowKeyPressed(Button button)
	{
		const UseNumpadAsArrowKeys mode = GetUseNumpadAsArrowKeysMode();
		if (mode == kOff)
		{
			return false;
		}

		switch (button)
		{
		case kButtonLeft:
			return KeyNum4.pressed();
		case kButtonUp:
			return mode == kOnKeyboard ? KeyNum8.pressed() : KeyNum2.pressed();
		case kButtonRight:
			return KeyNum6.pressed();
		case kButtonDown:
			return mode == kOnKeyboard ? KeyNum2.pressed() : KeyNum8.pressed();
		default:
			return false;
		}
	}

	bool IsNumpadArrowKeyDown(Button button)
	{
		const UseNumpadAsArrowKeys mode = GetUseNumpadAsArrowKeysMode();
		if (mode == kOff)
		{
			return false;
		}

		switch (button)
		{
		case kButtonLeft:
			return KeyNum4.down();
		case kButtonUp:
			return mode == kOnKeyboard ? KeyNum8.down() : KeyNum2.down();
		case kButtonRight:
			return KeyNum6.down();
		case kButtonDown:
			return mode == kOnKeyboard ? KeyNum2.down() : KeyNum8.down();
		default:
			return false;
		}
	}

	bool IsNumpadArrowKeyUp(Button button)
	{
		const UseNumpadAsArrowKeys mode = GetUseNumpadAsArrowKeysMode();
		if (mode == kOff)
		{
			return false;
		}

		switch (button)
		{
		case kButtonLeft:
			return KeyNum4.up();
		case kButtonUp:
			return mode == kOnKeyboard ? KeyNum8.up() : KeyNum2.up();
		case kButtonRight:
			return KeyNum6.up();
		case kButtonDown:
			return mode == kOnKeyboard ? KeyNum2.up() : KeyNum8.up();
		default:
			return false;
		}
	}

	Button SwapLaserButtonIfNeeded(Button button)
	{
		if (!ConfigIni::GetBool(ConfigIni::Key::kSwapLaserLR, false))
		{
			return button;
		}

		switch (button)
		{
		case kButtonLeftLaserL:
			return kButtonRightLaserL;
		case kButtonLeftLaserR:
			return kButtonRightLaserR;
		case kButtonRightLaserL:
			return kButtonLeftLaserL;
		case kButtonRightLaserR:
			return kButtonLeftLaserR;
		default:
			return button;
		}
	}

	const Input& GetConfigSetInputApplyingSwap(const ConfigSetArray& configSet, Button button)
	{
		return configSet[SwapLaserButtonIfNeeded(button)];
	}

	Input& GetConfigSetInputApplyingSwap(ConfigSetArray& configSet, Button button)
	{
		return configSet[SwapLaserButtonIfNeeded(button)];
	}

	void RevertUnconfigurableKeyConfigs()
	{
		// Keyboard 1の場合、ユーザーによって変更できない固定のキーコンフィグがあるので上書き
		s_configSetArray[KeyConfig::kKeyboard1][kButtonStart] = KeyEnter;
		s_configSetArray[KeyConfig::kKeyboard1][kButtonBack] = KeyEscape;
		s_configSetArray[KeyConfig::kKeyboard1][kButtonAutoPlay] = KeyF11;
		s_configSetArray[KeyConfig::kKeyboard1][kButtonUp] = KeyUp;
		s_configSetArray[KeyConfig::kKeyboard1][kButtonDown] = KeyDown;
		s_configSetArray[KeyConfig::kKeyboard1][kButtonLeft] = KeyLeft;
		s_configSetArray[KeyConfig::kKeyboard1][kButtonRight] = KeyRight;
		s_configSetArray[KeyConfig::kKeyboard1][kButtonBackspace] = KeyBackspace;
	}

	bool IsBT3PlusStartPressed()
	{
		if (!ConfigIni::GetBool(ConfigIni::Key::kUse3BTsPlusStartAsBack, true))
		{
			return false;
		}

		const bool ignoreKeyboard = ShouldIgnoreKeyboardInput();
		bool startPressed = false;
		for (int32 i = 0; i < kConfigSetEnumCount; ++i)
		{
			if (ignoreKeyboard && kConfigSetDeviceTypes[i] == InputDeviceType::Keyboard)
			{
				continue;
			}
			if (s_configSetArray[i][kButtonStart].pressed())
			{
				startPressed = true;
				break;
			}
		}
		if (!startPressed)
		{
			return false;
		}

		int32 btPressedCount = 0;
		for (int32 btIdx = kButtonBT_A; btIdx <= kButtonBT_D; ++btIdx)
		{
			for (int32 i = 0; i < kConfigSetEnumCount; ++i)
			{
				if (ignoreKeyboard && kConfigSetDeviceTypes[i] == InputDeviceType::Keyboard)
				{
					continue;
				}
				if (s_configSetArray[i][btIdx].pressed())
				{
					++btPressedCount;
					break;
				}
			}
		}

		return btPressedCount >= 3;
	}

	bool IsBT3PlusStartDown()
	{
		if (!IsBT3PlusStartPressed())
		{
			return false;
		}

		const bool ignoreKeyboard = ShouldIgnoreKeyboardInput();

		for (int32 i = 0; i < kConfigSetEnumCount; ++i)
		{
			if (ignoreKeyboard && kConfigSetDeviceTypes[i] == InputDeviceType::Keyboard)
			{
				continue;
			}
			if (s_configSetArray[i][kButtonStart].down())
			{
				return true;
			}
		}

		for (int32 btIdx = kButtonBT_A; btIdx <= kButtonBT_D; ++btIdx)
		{
			for (int32 i = 0; i < kConfigSetEnumCount; ++i)
			{
				if (ignoreKeyboard && kConfigSetDeviceTypes[i] == InputDeviceType::Keyboard)
				{
					continue;
				}
				if (s_configSetArray[i][btIdx].down())
				{
					return true;
				}
			}
		}

		return false;
	}

	bool IsBT3PlusStartUp()
	{
		if (IsBT3PlusStartPressed())
		{
			return false;
		}

		const bool ignoreKeyboard = ShouldIgnoreKeyboardInput();

		for (int32 i = 0; i < kConfigSetEnumCount; ++i)
		{
			if (ignoreKeyboard && kConfigSetDeviceTypes[i] == InputDeviceType::Keyboard)
			{
				continue;
			}
			if (s_configSetArray[i][kButtonStart].up())
			{
				return true;
			}
		}

		for (int32 btIdx = kButtonBT_A; btIdx <= kButtonBT_D; ++btIdx)
		{
			for (int32 i = 0; i < kConfigSetEnumCount; ++i)
			{
				if (ignoreKeyboard && kConfigSetDeviceTypes[i] == InputDeviceType::Keyboard)
				{
					continue;
				}
				if (s_configSetArray[i][btIdx].up())
				{
					return true;
				}
			}
		}

		return false;
	}

	std::pair<Button, Button> GetLaserButtons(int32 laneIdx)
	{
		if (laneIdx == 0)
		{
			return { kButtonLeftLaserL, kButtonLeftLaserR };
		}
		else
		{
			return { kButtonRightLaserL, kButtonRightLaserR };
		}
	}
}

void KeyConfig::SetConfigValueByCommaSeparated(ConfigSet targetConfigSet, StringView configValue)
{
	Array<String> values = String(configValue).split(U',');

	if (targetConfigSet < 0 || kConfigSetEnumCount <= targetConfigSet)
	{
		Logger << U"[ksm warning] Invalid key config target '{}'!"_fmt(static_cast<std::underlying_type_t<ConfigSet>>(targetConfigSet));
		return;
	}

	if (values.size() != kConfigurableButtonEnumCount)
	{
		Logger << U"[ksm warning] Key configuration ({}) is ignored because value count does not match! (expected:{}, actual:{})"_fmt(kConfigSetNames[targetConfigSet], static_cast<int32>(kConfigurableButtonEnumCount), values.size());
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
			throw Error(U"KeyConfig::SetConfigValue(): Could not parse kDefaultConfigValues!");
		}
	}

	for (int32 i = 0; i < kConfigurableButtonEnumCount; ++i)
	{
		const int32 code = intValues[i];
		if (code >= 0)
		{
			s_configSetArray[targetConfigSet][i] = Input(kConfigSetDeviceTypes[targetConfigSet], static_cast<uint8>(code));
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
		throw Error(U"Warning: Invalid key config target '{}'!"_fmt(static_cast<std::underlying_type_t<ConfigSet>>(targetConfigSet)));
	}

	if (button < 0 || kConfigurableButtonEnumCount <= button)
	{
		throw Error(U"Warning: Invalid key config button '{}'!"_fmt(static_cast<std::underlying_type_t<ConfigurableButton>>(button)));
	}

	s_configSetArray[targetConfigSet][button] = input;

	RevertUnconfigurableKeyConfigs();
}

const Input& KeyConfig::GetConfigValue(ConfigSet targetConfigSet, ConfigurableButton button)
{
	if (targetConfigSet < 0 || kConfigSetEnumCount <= targetConfigSet)
	{
		throw Error(U"Warning: Invalid key config target '{}'!"_fmt(static_cast<std::underlying_type_t<ConfigSet>>(targetConfigSet)));
	}

	if (button < 0 || kConfigurableButtonEnumCount <= button)
	{
		throw Error(U"Warning: Invalid key config button '{}'!"_fmt(static_cast<std::underlying_type_t<ConfigurableButton>>(button)));
	}

	return s_configSetArray[targetConfigSet][button];
}

#ifdef __APPLE__
void KeyConfig::UpdatePlatformKeyboard()
{
	for (size_t i = 0; i < kPlatformKeys.size(); ++i)
	{
		const bool pressed = KSMPlatformMacOS_IsKeyPressed(kPlatformKeys[i].nativeCode);
		s_platformKeyStates[i].update(pressed);
	}
}
#endif

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
	if (button == kUnspecifiedButton)
	{
		return false;
	}

	const bool ignoreKeyboard = ShouldIgnoreKeyboardInput();

	for (const auto& configSet : s_configSetArray)
	{
		const auto& input = GetConfigSetInputApplyingSwap(configSet, button);
		if (input.deviceType() == InputDeviceType::Keyboard)
		{
			if (ignoreKeyboard)
			{
				continue;
			}
#ifdef __APPLE__
			bool isPlatformKey = false;
			for (size_t i = 0; i < kPlatformKeys.size(); ++i)
			{
				if (input.code() == (kPlatformKeys[i].code - kPlatformKeyCodeOffset))
				{
					if (s_platformKeyStates[i].pressed())
					{
						return true;
					}
					isPlatformKey = true;
					break;
				}
			}
			if (!isPlatformKey && input.pressed())
			{
				return true;
			}
#else
			if (input.pressed())
			{
				return true;
			}
#endif
		}
		else if (input.pressed())
		{
			return true;
		}
	}

	// FXの場合はLR両押しキーの状態も反映
	if (button == kButtonFX_L || button == kButtonFX_R)
	{
		for (const auto& configSet : s_configSetArray)
		{
			const auto& fxLRInput = configSet[kButtonFX_LR];
			if (ignoreKeyboard && fxLRInput.deviceType() == InputDeviceType::Keyboard)
			{
				continue;
			}
			if (fxLRInput.pressed())
			{
				return true;
			}
		}
	}

	if (button == kButtonBack)
	{
		if (IsBT3PlusStartPressed())
		{
			return true;
		}
	}

	// 矢印キーの場合、Numpadキーの状態も確認
	if (button == kButtonUp || button == kButtonDown || button == kButtonLeft || button == kButtonRight)
	{
		if (!ignoreKeyboard && IsNumpadArrowKeyPressed(button))
		{
			return true;
		}
	}

	return false;
}

Optional<Button> KeyConfig::LastPressedLaserButton(Button button1, Button button2)
{
	assert(button1 == kButtonLeftLaserL || button1 == kButtonLeftLaserR || button1 == kButtonRightLaserL || button1 == kButtonRightLaserR);
	assert(button2 == kButtonLeftLaserL || button2 == kButtonLeftLaserR || button2 == kButtonRightLaserL || button2 == kButtonRightLaserR);

	Optional<Button> lastButton = none;
	Duration minDuration = Duration::zero();
	for (const auto& configSet : s_configSetArray)
	{
		for (Button button : { button1, button2 })
		{
			if (GetConfigSetInputApplyingSwap(configSet, button).pressed())
			{
				const Duration duration = GetConfigSetInputApplyingSwap(configSet, button).pressedDuration();
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
	if (button == kUnspecifiedButton)
	{
		return false;
	}

	const bool ignoreKeyboard = ShouldIgnoreKeyboardInput();

	for (const auto& configSet : s_configSetArray)
	{
		const auto& input = GetConfigSetInputApplyingSwap(configSet, button);
		if (input.deviceType() == InputDeviceType::Keyboard)
		{
			if (ignoreKeyboard)
			{
				continue;
			}
#ifdef __APPLE__
			bool isPlatformKey = false;
			for (size_t i = 0; i < kPlatformKeys.size(); ++i)
			{
				if (input.code() == (kPlatformKeys[i].code - kPlatformKeyCodeOffset))
				{
					if (s_platformKeyStates[i].down())
					{
						return true;
					}
					isPlatformKey = true;
					break;
				}
			}
			if (!isPlatformKey && input.down())
			{
				return true;
			}
#else
			if (input.down())
			{
				return true;
			}
#endif
		}
		else if (input.down())
		{
			return true;
		}
	}

	// FXの場合はLR両押しキーの状態も反映
	if (button == kButtonFX_L || button == kButtonFX_R)
	{
		for (const auto& configSet : s_configSetArray)
		{
			const auto& fxLRInput = configSet[kButtonFX_LR];
			if (ignoreKeyboard && fxLRInput.deviceType() == InputDeviceType::Keyboard)
			{
				continue;
			}
			if (fxLRInput.down())
			{
				return true;
			}
		}
	}

	if (button == kButtonBack)
	{
		if (IsBT3PlusStartDown())
		{
			return true;
		}
	}

	// 矢印キーの場合、Numpadキーの状態も確認
	if (button == kButtonUp || button == kButtonDown || button == kButtonLeft || button == kButtonRight)
	{
		if (!ignoreKeyboard && IsNumpadArrowKeyDown(button))
		{
			return true;
		}
	}

	return false;
}

void KeyConfig::ClearInput(Button button)
{
	if (button == kUnspecifiedButton)
	{
		return;
	}

	for (auto& configSet : s_configSetArray)
	{
		GetConfigSetInputApplyingSwap(configSet, button).clearInput();
	}

	// FXの場合はLR両押しキーの状態もクリア
	if (button == kButtonFX_L || button == kButtonFX_R)
	{
		for (auto& configSet : s_configSetArray)
		{
			configSet[kButtonFX_LR].clearInput();
		}
	}
}

Co::Task<void> KeyConfig::WaitUntilDown(Button button)
{
	// 前シーンの入力で多重に反応しないよう、入力をクリア
	ClearInput(button);

	return Co::WaitUntil([button]() { return Down(button); });
}

bool KeyConfig::Up(Button button)
{
	if (button == kUnspecifiedButton)
	{
		return false;
	}

	// あるボタンを離したと判定するには、そのボタンに対応する全キーを離している必要がある
	if (Pressed(button))
	{
		return false;
	}

	const bool ignoreKeyboard = ShouldIgnoreKeyboardInput();

	for (const auto& configSet : s_configSetArray)
	{
		const auto& input = GetConfigSetInputApplyingSwap(configSet, button);
		if (input.deviceType() == InputDeviceType::Keyboard)
		{
			if (ignoreKeyboard)
			{
				continue;
			}
#ifdef __APPLE__
			bool isPlatformKey = false;
			for (size_t i = 0; i < kPlatformKeys.size(); ++i)
			{
				if (input.code() == (kPlatformKeys[i].code - kPlatformKeyCodeOffset))
				{
					if (s_platformKeyStates[i].up())
					{
						return true;
					}
					isPlatformKey = true;
					break;
				}
			}
			if (!isPlatformKey && input.up())
			{
				return true;
			}
#else
			if (input.up())
			{
				return true;
			}
#endif
		}
		else if (input.up())
		{
			return true;
		}
	}

	if (button == kButtonBack)
	{
		if (IsBT3PlusStartUp())
		{
			return true;
		}
	}

	// 矢印キーの場合、Numpadキーの状態も確認
	if (button == kButtonUp || button == kButtonDown || button == kButtonLeft || button == kButtonRight)
	{
		if (!ignoreKeyboard && IsNumpadArrowKeyUp(button))
		{
			return true;
		}
	}

	return false;
}

bool KeyConfig::IsLaserInputDigital()
{
	const int32 laserInputType = ConfigIni::GetInt(ConfigIni::Key::kLaserInputType, ConfigIni::Value::LaserInputType::kKeyboard);
	return laserInputType == ConfigIni::Value::LaserInputType::kKeyboard;
}

double KeyConfig::LaserDeltaCursorX(int32 laneIdx, double deltaTimeSec)
{
	// 入力方式が初期化されていない、または設定が変更された場合は再初期化
	if (!s_laserInputMethods[laneIdx] || s_laserInputMethods[laneIdx]->reconstructionNeeded())
	{
		const int32 laserInputType = ConfigIni::GetInt(ConfigIni::Key::kLaserInputType, ConfigIni::Value::LaserInputType::kKeyboard);
		if (laserInputType == ConfigIni::Value::LaserInputType::kSlider)
		{
			s_laserInputMethods[laneIdx] = std::make_unique<SliderLaserInput>(laneIdx);
		}
		else if (laserInputType == ConfigIni::Value::LaserInputType::kAnalogStickXY)
		{
			s_laserInputMethods[laneIdx] = std::make_unique<AnalogStickXYLaserInput>(laneIdx);
		}
		else if (laserInputType == ConfigIni::Value::LaserInputType::kMouseXY)
		{
			s_laserInputMethods[laneIdx] = std::make_unique<MouseXYLaserInput>(laneIdx);
		}
		else
		{
			const auto [buttonL, buttonR] = GetLaserButtons(laneIdx);
			s_laserInputMethods[laneIdx] = std::make_unique<KeyboardLaserInput>(buttonL, buttonR);
		}
	}

	return s_laserInputMethods[laneIdx]->getDeltaCursorX(deltaTimeSec);
}
