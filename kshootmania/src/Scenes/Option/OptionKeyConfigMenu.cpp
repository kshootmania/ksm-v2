#include "OptionKeyConfigMenu.hpp"

#ifdef __APPLE__
#include <ksmplatform_macos/input_method.h>
#endif

namespace
{
	enum class Direction
	{
		Up,
		Down,
		Left,
		Right,
	};

	OptionKeyConfigCursor NextCursor(OptionKeyConfigCursor cursor, Direction direction)
	{
		if (cursor == OptionKeyConfigCursor::BT_A)
		{
			switch (direction)
			{
			case Direction::Up:
				return OptionKeyConfigCursor::Laser_L;
			case Direction::Down:
				return OptionKeyConfigCursor::FX_L;
			case Direction::Left:
				return OptionKeyConfigCursor::BT_A;
			case Direction::Right:
				return OptionKeyConfigCursor::BT_B;
			default:
				break;
			}
		}
		else if (cursor == OptionKeyConfigCursor::BT_B)
		{
			switch (direction)
			{
			case Direction::Up:
				return OptionKeyConfigCursor::Back;
			case Direction::Down:
				return OptionKeyConfigCursor::FX_L;
			case Direction::Left:
				return OptionKeyConfigCursor::BT_A;
			case Direction::Right:
				return OptionKeyConfigCursor::BT_C;
			default:
				break;
			}
		}
		else if (cursor == OptionKeyConfigCursor::BT_C)
		{
			switch (direction)
			{
			case Direction::Up:
				return OptionKeyConfigCursor::Back;
			case Direction::Down:
				return OptionKeyConfigCursor::FX_R;
			case Direction::Left:
				return OptionKeyConfigCursor::BT_B;
			case Direction::Right:
				return OptionKeyConfigCursor::BT_D;
			default:
				break;
			}
		}
		else if (cursor == OptionKeyConfigCursor::BT_D)
		{
			switch (direction)
			{
			case Direction::Up:
				return OptionKeyConfigCursor::Laser_R;
			case Direction::Down:
				return OptionKeyConfigCursor::FX_R;
			case Direction::Left:
				return OptionKeyConfigCursor::BT_C;
			case Direction::Right:
				return OptionKeyConfigCursor::BT_D;
			default:
				break;
			}
		}
		else if (cursor == OptionKeyConfigCursor::FX_L)
		{
			switch (direction)
			{
			case Direction::Up:
				return OptionKeyConfigCursor::BT_A;
			case Direction::Down:
				return OptionKeyConfigCursor::ConfigSet;
			case Direction::Left:
				return OptionKeyConfigCursor::FX_L;
			case Direction::Right:
				return OptionKeyConfigCursor::FX_R;
			default:
				break;
			}
		}
		else if (cursor == OptionKeyConfigCursor::FX_R)
		{
			switch (direction)
			{
			case Direction::Up:
				return OptionKeyConfigCursor::BT_D;
			case Direction::Down:
				return OptionKeyConfigCursor::ConfigSet;
			case Direction::Left:
				return OptionKeyConfigCursor::FX_L;
			case Direction::Right:
				return OptionKeyConfigCursor::FX_LR;
			default:
				break;
			}
		}
		else if (cursor == OptionKeyConfigCursor::FX_LR)
		{
			switch (direction)
			{
			case Direction::Up:
				return OptionKeyConfigCursor::BT_D;
			case Direction::Down:
				return OptionKeyConfigCursor::FX_LR;
			case Direction::Left:
				return OptionKeyConfigCursor::FX_R;
			case Direction::Right:
				return OptionKeyConfigCursor::FX_LR;
			default:
				break;
			}
		}
		else if (cursor == OptionKeyConfigCursor::Laser_L)
		{
			switch (direction)
			{
			case Direction::Up:
				return OptionKeyConfigCursor::Laser_L;
			case Direction::Down:
				return OptionKeyConfigCursor::BT_A;
			case Direction::Left:
				return OptionKeyConfigCursor::Laser_L;
			case Direction::Right:
				return OptionKeyConfigCursor::Start;
			default:
				break;
			}
		}
		else if (cursor == OptionKeyConfigCursor::Laser_R)
		{
			switch (direction)
			{
			case Direction::Up:
				return OptionKeyConfigCursor::Laser_R;
			case Direction::Down:
				return OptionKeyConfigCursor::BT_D;
			case Direction::Left:
				return OptionKeyConfigCursor::Start;
			case Direction::Right:
				return OptionKeyConfigCursor::Laser_R;
			default:
				break;
			}
		}
		else if (cursor == OptionKeyConfigCursor::Start)
		{
			switch (direction)
			{
			case Direction::Up:
				return OptionKeyConfigCursor::Start;
			case Direction::Down:
				return OptionKeyConfigCursor::Back;
			case Direction::Left:
				return OptionKeyConfigCursor::Laser_L;
			case Direction::Right:
				return OptionKeyConfigCursor::Laser_R;
			default:
				break;
			}
		}
		else if (cursor == OptionKeyConfigCursor::Back)
		{
			switch (direction)
			{
			case Direction::Up:
				return OptionKeyConfigCursor::Start;
			case Direction::Down:
				return OptionKeyConfigCursor::BT_B;
			case Direction::Left:
				return OptionKeyConfigCursor::Laser_L;
			case Direction::Right:
				return OptionKeyConfigCursor::Laser_R;
			default:
				break;
			}
		}
		else if (cursor == OptionKeyConfigCursor::ConfigSet)
		{
			switch (direction)
			{
			case Direction::Up:
				return OptionKeyConfigCursor::FX_L;
			case Direction::Down:
				return OptionKeyConfigCursor::ConfigSet;
			case Direction::Left:
				return OptionKeyConfigCursor::ConfigSet;
			case Direction::Right:
				return OptionKeyConfigCursor::ConfigSet;
			default:
				break;
			}
		}

		return cursor;
	}

	Optional<ConfigurableButton> CursorToButton1(OptionKeyConfigCursor cursor)
	{
		switch (cursor)
		{
		case OptionKeyConfigCursor::BT_A:
			return kButtonBT_A;
		case OptionKeyConfigCursor::BT_B:
			return kButtonBT_B;
		case OptionKeyConfigCursor::BT_C:
			return kButtonBT_C;
		case OptionKeyConfigCursor::BT_D:
			return kButtonBT_D;
		case OptionKeyConfigCursor::FX_L:
			return kButtonFX_L;
		case OptionKeyConfigCursor::FX_R:
			return kButtonFX_R;
		case OptionKeyConfigCursor::FX_LR:
			return kButtonFX_LR;
		case OptionKeyConfigCursor::Laser_L:
			return kButtonLeftLaserL;
		case OptionKeyConfigCursor::Laser_R:
			return kButtonRightLaserL;
		case OptionKeyConfigCursor::Start:
			return kButtonStart;
		case OptionKeyConfigCursor::Back:
			return kButtonBack;
		default:
			return none;
		}
	}

	Optional<ConfigurableButton> CursorToButton2(OptionKeyConfigCursor cursor)
	{
		switch (cursor)
		{
		case OptionKeyConfigCursor::Laser_L:
			return kButtonLeftLaserR;
		case OptionKeyConfigCursor::Laser_R:
			return kButtonRightLaserR;
		default:
			return none;
		}
	}

	String KeyCodeToString(uint8 keyCode)
	{
		if (keyCode == 0x00)
		{
			return String{ I18n::Get(I18n::Option::KeyConfigNoAssign) };
		}
#ifdef __APPLE__
		// macOSプラットフォームキーのチェック
		for (const auto& platformKey : kPlatformKeys)
		{
			if (keyCode == (platformKey.code - kPlatformKeyCodeOffset))
			{
				if (platformKey.nativeCode == 0x66)
				{
					return U"英数";
				}
				else if (platformKey.nativeCode == 0x68)
				{
					return U"かな";
				}
			}
		}
#endif
		if (keyCode == 0x08)
		{
			return U"BackSpace";
		}
		if (keyCode == 0x09)
		{
			return U"Tab";
		}
		if (keyCode == 0x0C)
		{
			return U"Clear";
		}
		if (keyCode == 0x0D)
		{
			return U"Enter";
		}
		if (keyCode == 0x10)
		{
			return U"Shift";
		}
		if (keyCode == 0x11)
		{
			return U"Ctrl";
		}
		if (keyCode == 0x12)
		{
			return U"Alt";
		}
		if (keyCode == 0x13)
		{
			return U"Pause";
		}
		if (keyCode == 0x14)
		{
			return U"CapsLock";
		}
		if (keyCode == 0x15)
		{
			return U"かな";
		}
		if (keyCode == 0x17)
		{
			return U"Junja";
		}
		if (keyCode == 0x18)
		{
			return U"Final";
		}
		if (keyCode == 0x19)
		{
			return U"漢字";
		}
		if (keyCode == 0x1B)
		{
			return U"Esc";
		}
		if (keyCode == 0x1C)
		{
			return U"変換";
		}
		if (keyCode == 0x1D)
		{
			return U"無変換";
		}
		if (keyCode == 0x1E)
		{
			return U"Accept";
		}
		if (keyCode == 0x1F)
		{
			return U"Mode";
		}
		if (keyCode == 0x20)
		{
			return U"Space";
		}
		if (keyCode == 0x21)
		{
			return U"PgUp";
		}
		if (keyCode == 0x22)
		{
			return U"PgDn";
		}
		if (keyCode == 0x23)
		{
			return U"End";
		}
		if (keyCode == 0x24)
		{
			return U"Home";
		}
		if (keyCode == 0x25)
		{
			return U"Left";
		}
		if (keyCode == 0x26)
		{
			return U"Up";
		}
		if (keyCode == 0x27)
		{
			return U"Right";
		}
		if (keyCode == 0x28)
		{
			return U"Down";
		}
		if (keyCode == 0x2D)
		{
			return U"Insert";
		}
		if (keyCode == 0x2E)
		{
			return U"Delete";
		}
		if (keyCode == 0x30)
		{
			return U"0";
		}
		if (keyCode == 0x31)
		{
			return U"1";
		}
		if (keyCode == 0x32)
		{
			return U"2";
		}
		if (keyCode == 0x33)
		{
			return U"3";
		}
		if (keyCode == 0x34)
		{
			return U"4";
		}
		if (keyCode == 0x35)
		{
			return U"5";
		}
		if (keyCode == 0x36)
		{
			return U"6";
		}
		if (keyCode == 0x37)
		{
			return U"7";
		}
		if (keyCode == 0x38)
		{
			return U"8";
		}
		if (keyCode == 0x39)
		{
			return U"9";
		}
		if (keyCode == 0x41)
		{
			return U"A";
		}
		if (keyCode == 0x42)
		{
			return U"B";
		}
		if (keyCode == 0x43)
		{
			return U"C";
		}
		if (keyCode == 0x44)
		{
			return U"D";
		}
		if (keyCode == 0x45)
		{
			return U"E";
		}
		if (keyCode == 0x46)
		{
			return U"F";
		}
		if (keyCode == 0x47)
		{
			return U"G";
		}
		if (keyCode == 0x48)
		{
			return U"H";
		}
		if (keyCode == 0x49)
		{
			return U"I";
		}
		if (keyCode == 0x4A)
		{
			return U"J";
		}
		if (keyCode == 0x4B)
		{
			return U"K";
		}
		if (keyCode == 0x4C)
		{
			return U"L";
		}
		if (keyCode == 0x4D)
		{
			return U"M";
		}
		if (keyCode == 0x4E)
		{
			return U"N";
		}
		if (keyCode == 0x4F)
		{
			return U"O";
		}
		if (keyCode == 0x50)
		{
			return U"P";
		}
		if (keyCode == 0x51)
		{
			return U"Q";
		}
		if (keyCode == 0x52)
		{
			return U"R";
		}
		if (keyCode == 0x53)
		{
			return U"S";
		}
		if (keyCode == 0x54)
		{
			return U"T";
		}
		if (keyCode == 0x55)
		{
			return U"U";
		}
		if (keyCode == 0x56)
		{
			return U"V";
		}
		if (keyCode == 0x57)
		{
			return U"W";
		}
		if (keyCode == 0x58)
		{
			return U"X";
		}
		if (keyCode == 0x59)
		{
			return U"Y";
		}
		if (keyCode == 0x5A)
		{
			return U"Z";
		}
		if (keyCode == 0x5B)
		{
			return U"Win";
		}
		if (keyCode == 0x60)
		{
			return U"0";
		}
		if (keyCode == 0x61)
		{
			return U"1";
		}
		if (keyCode == 0x62)
		{
			return U"2";
		}
		if (keyCode == 0x63)
		{
			return U"3";
		}
		if (keyCode == 0x64)
		{
			return U"4";
		}
		if (keyCode == 0x65)
		{
			return U"5";
		}
		if (keyCode == 0x66)
		{
			return U"6";
		}
		if (keyCode == 0x67)
		{
			return U"7";
		}
		if (keyCode == 0x68)
		{
			return U"8";
		}
		if (keyCode == 0x69)
		{
			return U"9";
		}
		if (keyCode == 0x6A)
		{
			return U"*";
		}
		if (keyCode == 0x6B)
		{
			return U";";
		}
		if (keyCode == 0x6C)
		{
			return U",";
		}
		if (keyCode == 0x6D)
		{
			return U"-";
		}
		if (keyCode == 0x6E)
		{
			return U".";
		}
		if (keyCode == 0x6F)
		{
			return U"/";
		}
		if (keyCode == 0x70)
		{
			return U"F1";
		}
		if (keyCode == 0x71)
		{
			return U"F2";
		}
		if (keyCode == 0x72)
		{
			return U"F3";
		}
		if (keyCode == 0x73)
		{
			return U"F4";
		}
		if (keyCode == 0x74)
		{
			return U"F5";
		}
		if (keyCode == 0x75)
		{
			return U"F6";
		}
		if (keyCode == 0x76)
		{
			return U"F7";
		}
		if (keyCode == 0x77)
		{
			return U"F8";
		}
		if (keyCode == 0x78)
		{
			return U"F9";
		}
		if (keyCode == 0x79)
		{
			return U"F10";
		}
		if (keyCode == 0x7A)
		{
			return U"F11";
		}
		if (keyCode == 0x7B)
		{
			return U"F12";
		}
		if (keyCode == 0x7C)
		{
			return U"F13";
		}
		if (keyCode == 0x7D)
		{
			return U"F14";
		}
		if (keyCode == 0x7E)
		{
			return U"F15";
		}
		if (keyCode == 0x7F)
		{
			return U"F16";
		}
		if (keyCode == 0x80)
		{
			return U"F17";
		}
		if (keyCode == 0x81)
		{
			return U"F18";
		}
		if (keyCode == 0x82)
		{
			return U"F19";
		}
		if (keyCode == 0x83)
		{
			return U"F20";
		}
		if (keyCode == 0x84)
		{
			return U"F21";
		}
		if (keyCode == 0x85)
		{
			return U"F22";
		}
		if (keyCode == 0x86)
		{
			return U"F23";
		}
		if (keyCode == 0x87)
		{
			return U"F24";
		}
		if (keyCode == 0x90)
		{
			return U"Num0";
		}
		if (keyCode == 0x91)
		{
			return U"Num1";
		}
		if (keyCode == 0x92)
		{
			return U"Num2";
		}
		if (keyCode == 0x93)
		{
			return U"Num3";
		}
		if (keyCode == 0x94)
		{
			return U"Num4";
		}
		if (keyCode == 0x95)
		{
			return U"Num5";
		}
		if (keyCode == 0x96)
		{
			return U"Num6";
		}
		if (keyCode == 0x97)
		{
			return U"Num7";
		}
		if (keyCode == 0x98)
		{
			return U"Num8";
		}
		if (keyCode == 0x99)
		{
			return U"Num9";
		}
		if (keyCode == 0xA0)
		{
			return U"LeftShift";
		}
		if (keyCode == 0xA1)
		{
			return U"RightShift";
		}
		if (keyCode == 0xA2)
		{
			return U"LeftControl";
		}
		if (keyCode == 0xA3)
		{
			return U"RightControl";
		}
		if (keyCode == 0xA4)
		{
			return U"LeftAlt";
		}
		if (keyCode == 0xA5)
		{
			return U"RightAlt";
		}
		if (keyCode == 0xA6)
		{
			return U"BrowserBack";
		}
		if (keyCode == 0xA7)
		{
			return U"BrowserForward";
		}
		if (keyCode == 0xA8)
		{
			return U"BrowserRefresh";
		}
		if (keyCode == 0xA9)
		{
			return U"BrowserStop";
		}
		if (keyCode == 0xAA)
		{
			return U"BrowserSearch";
		}
		if (keyCode == 0xAB)
		{
			return U"BrowserFavorites";
		}
		if (keyCode == 0xAC)
		{
			return U"BrowserHome";
		}
		if (keyCode == 0xAD)
		{
			return U"VolumeMute";
		}
		if (keyCode == 0xAE)
		{
			return U"VolumeDown";
		}
		if (keyCode == 0xAF)
		{
			return U"VolumeUp";
		}
		if (keyCode == 0xB0)
		{
			return U"MediaNextTrack";
		}
		if (keyCode == 0xB1)
		{
			return U"MediaPrevTrack";
		}
		if (keyCode == 0xB2)
		{
			return U"MediaStop";
		}
		if (keyCode == 0xB3)
		{
			return U"MediaPlayPause";
		}
		if (keyCode == 0xB4)
		{
			return U"LaunchMail";
		}
		if (keyCode == 0xB5)
		{
			return U"SelectMedia";
		}
		if (keyCode == 0xB6)
		{
			return U"LaunchApp1";
		}
		if (keyCode == 0xB7)
		{
			return U"LaunchApp2";
		}
		if (keyCode == 0xBA)
		{
			return U":";
		}
		if (keyCode == 0xBB)
		{
			return U";";
		}
		if (keyCode == 0xBC)
		{
			return U",";
		}
		if (keyCode == 0xBD)
		{
			return U"-";
		}
		if (keyCode == 0xBE)
		{
			return U".";
		}
		if (keyCode == 0xBF)
		{
			return U"/";
		}
		if (keyCode == 0xC0)
		{
			return U"@";
		}
		if (keyCode == 0xDB)
		{
			return U"[";
		}
		if (keyCode == 0xDC)
		{
			return U"\\";
		}
		if (keyCode == 0xDD)
		{
			return U"]";
		}
		if (keyCode == 0xDE)
		{
			return U"^";
		}
		if (keyCode == 0xDF)
		{
			return U"_";
		}
		if (keyCode == 0xE2)
		{
			return U"\\";
		}
		if (keyCode == 0xF6)
		{
			return U"Attn";
		}
		if (keyCode == 0xF7)
		{
			return U"Crsel";
		}
		if (keyCode == 0xF8)
		{
			return U"Exsel";
		}
		if (keyCode == 0xF9)
		{
			return U"EraseEof";
		}
		if (keyCode == 0xFA)
		{
			return U"Play";
		}
		if (keyCode == 0xFB)
		{
			return U"Zoom";
		}
		if (keyCode == 0xFC)
		{
			return U"PA1";
		}
		if (keyCode == 0xFD)
		{
			return U"OemClear";
		}
		else
		{
			// 16進数で表示する
			return U"({:02X})"_fmt(keyCode);
		}
	}

	String GamepadButtonToString(const Input& input)
	{
		// デバイス番号(1始まり)-ボタン番号
		// TODO: 一旦v1と類似仕様にしているが、playerIndexが不定の可能性があるためvendorId,productID等を使うよう要改善
		return I18n::Get(I18n::Option::KeyConfigGamepadButtonFormat, input.playerIndex() + 1, input.code());
	}

	String InputToString(const Input& input)
	{
		if (input.deviceType() == InputDeviceType::Undefined)
		{
			return String{ I18n::Get(I18n::Option::KeyConfigNoAssign) };
		}
		else if (input.deviceType() == InputDeviceType::Gamepad)
		{
			return GamepadButtonToString(input);
		}
		else if (input.deviceType() == InputDeviceType::Keyboard)
		{
			return KeyCodeToString(input.code());
		}
		else
		{
			return U"?";
		}
	}

	Texture CreateFXLRTexture()
	{
		Image image{ U"imgs/config_fx.gif" };

		// 黒を透明にする
		for (Color& colorRef : image)
		{
			if (colorRef == Palette::Black)
			{
				colorRef = Color{ 255, 0 };
			}
		}

		return Texture{ image };
	}

	// 指定したConfigurableButtonのキーが他のボタンと重複しているかチェック
	bool HasDuplicateKey(KeyConfig::ConfigSet targetConfigSet, ConfigurableButton targetButton)
	{
		const Input& targetInput = KeyConfig::GetConfigValue(targetConfigSet, targetButton);

		// 未割り当て(code == 0)の場合は重複扱いにしない
		if (targetInput.deviceType() == InputDeviceType::Undefined || targetInput.code() == 0)
		{
			return false;
		}

		// 全てのConfigSetについてチェック
		for (int32 configSetIdx = 0; configSetIdx < KeyConfig::kConfigSetEnumCount; ++configSetIdx)
		{
			const auto configSet = static_cast<KeyConfig::ConfigSet>(configSetIdx);

			for (int32 i = 0; i < kConfigurableButtonEnumCount; ++i)
			{
				// 自分自身は除外
				if (configSet == targetConfigSet && i == targetButton)
				{
					continue;
				}

				const Input& otherInput = KeyConfig::GetConfigValue(configSet, static_cast<ConfigurableButton>(i));
				if (otherInput.deviceType() == targetInput.deviceType() && otherInput.code() == targetInput.code())
				{
					return true;
				}
			}
		}

		return false;
	}
}

KeyConfig::ConfigSet OptionKeyConfigMenu::targetConfigSet() const
{
	return m_configSetMenu.cursorAs<KeyConfig::ConfigSet>();
}

void OptionKeyConfigMenu::updateNoneState()
{
	m_horizontalCursorInput.update();
	m_verticalCursorInput.update();

	Optional<Direction> direction;
	if (m_horizontalCursorInput.deltaCursor() < 0)
	{
		direction = Direction::Left;
	}
	else if (m_horizontalCursorInput.deltaCursor() > 0)
	{
		direction = Direction::Right;
	}
	else if (m_verticalCursorInput.deltaCursor() < 0)
	{
		direction = Direction::Up;
	}
	else if (m_verticalCursorInput.deltaCursor() > 0)
	{
		direction = Direction::Down;
	}

	if (direction.has_value())
	{
		m_cursor = NextCursor(m_cursor, direction.value());
	}

	// 設定解除
	if ((KeyConfig::Pressed(kButtonFX_L) && KeyConfig::Down(kButtonFX_R)) || (KeyConfig::Down(kButtonFX_L) && KeyConfig::Pressed(kButtonFX_R)) || KeyConfig::Down(kButtonFX_LR) || KeySpace.down())
	{
		const auto button1 = CursorToButton1(m_cursor);
		if (button1.has_value())
		{
			KeyConfig::SetConfigValue(targetConfigSet(), button1.value(), Input{});
		}
		const auto button2 = CursorToButton2(m_cursor);
		if (button2.has_value())
		{
			KeyConfig::SetConfigValue(targetConfigSet(), button2.value(), Input{});
		}
		KeyConfig::SaveToConfigIni();
	}

	// 選択
	if (KeyConfig::Down(kButtonStart) && CursorToButton1(m_cursor).has_value())
	{
		m_state = OptionKeyConfigMenuState::SettingButton1;
	}

	// ConfigSet切り替え
	if (m_cursor == OptionKeyConfigCursor::ConfigSet)
	{
		m_configSetMenu.update();
	}
}

void OptionKeyConfigMenu::setInput(const Input& input)
{
	if (m_state == OptionKeyConfigMenuState::None)
	{
		Print << U"Warning: setInput is called while state is None";
	}
	else if (m_state == OptionKeyConfigMenuState::SettingButton1)
	{
		const auto button = CursorToButton1(m_cursor);
		if (button.has_value())
		{
			KeyConfig::SetConfigValue(targetConfigSet(), button.value(), input);
			KeyConfig::SaveToConfigIni();

			// 設定時のキー押下が反応しないよう入力クリア
			KeyConfig::ClearInput(button.value());

			if (CursorToButton2(m_cursor).has_value())
			{
				m_state = OptionKeyConfigMenuState::SettingButton2;
			}
			else
			{
				m_state = OptionKeyConfigMenuState::None;
			}
		}
		else
		{
			Print << U"Warning: setInput is called while cursor cannot be converted to button1";
		}
	}
	else if (m_state == OptionKeyConfigMenuState::SettingButton2)
	{
		const auto button = CursorToButton2(m_cursor);
		if (button.has_value())
		{
			KeyConfig::SetConfigValue(targetConfigSet(), button.value(), input);
			KeyConfig::SaveToConfigIni();

			// 設定時のキー押下が反応しないよう入力クリア
			KeyConfig::ClearInput(button.value());

			m_state = OptionKeyConfigMenuState::None;
		}
		else
		{
			Print << U"Warning: setInput is called while cursor cannot be converted to button2";
		}
	}
}

void OptionKeyConfigMenu::updateSettingButtonState()
{
	if (targetConfigSet() == KeyConfig::ConfigSet::kKeyboard1 || targetConfigSet() == KeyConfig::ConfigSet::kKeyboard2)
	{
		const Array<Input> keys = Keyboard::GetAllInputs();
		for (const auto& key : keys)
		{
			if (key.down())
			{
				setInput(key);
				break;
			}
		}

#ifdef __APPLE__
		// macOSプラットフォーム特有のキーもチェック
		for (const auto& key : kPlatformKeys)
		{
			const bool pressed = KSMPlatformMacOS_IsKeyPressed(key.nativeCode);
			const bool wasPressed = m_platformKeyWasPressed[key.nativeCode];

			if (pressed && !wasPressed)
			{
				setInput(Input(InputDeviceType::Keyboard, key.code));
			}
			m_platformKeyWasPressed[key.nativeCode] = pressed;
		}
#endif
	}
	else
	{
		// スペースキーで設定解除
		if (KeySpace.down())
		{
			setInput(Input{});
			return;
		}

		for (int32 playerIdx = 0; playerIdx < static_cast<int32>(Gamepad.MaxPlayerCount); ++playerIdx)
		{
			if (const auto& gamepad = Gamepad(playerIdx)) // 接続されている場合のみ
			{
				bool isAnyButtonDown = false;
				for (const auto& input : gamepad.buttons)
				{
					if (input.down())
					{
						setInput(input);
						isAnyButtonDown = true;
						break;
					}
				}
				if (isAnyButtonDown)
				{
					break;
				}
			}
		}
	}
}

OptionKeyConfigMenu::OptionKeyConfigMenu()
	: m_horizontalCursorInput(CursorInput::CreateInfo
		{
			.type = CursorInput::Type::Horizontal,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		})
	, m_verticalCursorInput(CursorInput::CreateInfo
		{
			.type = CursorInput::Type::Vertical,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		})
	, m_configSetMenu(LinearMenu::CreateInfoWithEnumCount
		{
			.cursorInputCreateInfo = CursorInput::CreateInfo
			{
				.type = CursorInput::Type::Horizontal,
				.buttonFlags = CursorButtonFlags::kArrowOrLaser,
			},
			.enumCount = KeyConfig::kConfigSetEnumCount,
			.cyclic = IsCyclicMenuYN::No,
		})
	, m_fxLRTexture(CreateFXLRTexture(), TiledTextureSizeInfo
		{
			.column = 3,
			.sourceScale = SourceScale::k1x,
			.sourceSize = { 192, 192 },
		})
{
}

void OptionKeyConfigMenu::update()
{
	switch (m_state)
	{
	case OptionKeyConfigMenuState::None:
		updateNoneState();
		break;

	case OptionKeyConfigMenuState::SettingButton1:
	case OptionKeyConfigMenuState::SettingButton2:
		updateSettingButtonState();
		break;

	default:
		break;
	}
}

void OptionKeyConfigMenu::draw() const
{
	// BT
	for (int32 i = 0; i < kson::kNumBTLanes; ++i)
	{
		const Rect rect{ LeftMargin() + Scaled(95 + i * 120), Scaled(165), Scaled(100), Scaled(100) };
		const auto matchCursor = static_cast<OptionKeyConfigCursor>(static_cast<int32>(OptionKeyConfigCursor::BT_A) + i);
		if (m_cursor == matchCursor)
		{
			const Color frameColor = m_state == OptionKeyConfigMenuState::SettingButton1 ? Palette::Yellow : Palette::Red;
			rect.drawFrame(Scaled(5), Scaled(5), frameColor);
		}
		rect.draw(Palette::White);

		const auto button = static_cast<ConfigurableButton>(kButtonBT_A + i);
		const Input& input = KeyConfig::GetConfigValue(targetConfigSet(), button);
		const Color textColor = HasDuplicateKey(targetConfigSet(), button) ? Palette::Red : Palette::Black;
		m_font(InputToString(input)).drawAt(Scaled(16), rect.center(), textColor);
	}

	// FX
	for (int32 i = 0; i < kson::kNumFXLanes; ++i)
	{
		const Rect rect{ LeftMargin() + Scaled(185 + i * 150), Scaled(290), Scaled(130), Scaled(60) };
		const auto matchCursor = static_cast<OptionKeyConfigCursor>(static_cast<int32>(OptionKeyConfigCursor::FX_L) + i);
		if (m_cursor == matchCursor)
		{
			const Color frameColor = m_state == OptionKeyConfigMenuState::SettingButton1 ? Palette::Yellow : Palette::Red;
			rect.drawFrame(Scaled(5), Scaled(5), frameColor);
		}
		rect.draw(Color{ 96, 96, 96 });

		const auto button = static_cast<ConfigurableButton>(kButtonFX_L + i);
		const Input& input = KeyConfig::GetConfigValue(targetConfigSet(), button);
		const Color textColor = HasDuplicateKey(targetConfigSet(), button) ? Palette::Red : Palette::White;
		m_font(InputToString(input)).drawAt(Scaled(16), rect.center(), textColor);
	}

	// FX-L+R
	{
		const Point position{ LeftMargin() + Scaled(540), Scaled(324) };

		const auto matchCursor = OptionKeyConfigCursor::FX_LR;
		int32 column;
		if (m_cursor == matchCursor)
		{
			column = m_state == OptionKeyConfigMenuState::SettingButton1 ? 2 : 1;
		}
		else
		{
			column = 0;
		}
		m_fxLRTexture(0, column).resized(Scaled(96, 96)).drawAt(position);

		const Input& input = KeyConfig::GetConfigValue(targetConfigSet(), kButtonFX_LR);
		const Color textColor = HasDuplicateKey(targetConfigSet(), kButtonFX_LR) ? Palette::Red : Palette::White;
		m_font(InputToString(input)).drawAt(Scaled(16), position + Scaled(0, 58), textColor);
	}

	// Laser
	for (int32 i = 0; i < kson::kNumLaserLanes; ++i)
	{
		const Circle circle{ Point{ LeftMargin() + Scaled(165 + i * 320), Scaled(110) }, Scaled(30) };
		const auto matchCursor = static_cast<OptionKeyConfigCursor>(static_cast<int32>(OptionKeyConfigCursor::Laser_L) + i);
		if (m_cursor == matchCursor)
		{
			const Color frameColor =
				m_state == OptionKeyConfigMenuState::SettingButton1 ? Color{ 128, 255, 0 } :
				m_state == OptionKeyConfigMenuState::SettingButton2 ? Color{ 255, 128, 0 } : Palette::Red;
			circle.drawFrame(Scaled(5), Scaled(5), frameColor);
		}
		const Color fillColor = i == 0 ? Color{ 64, 96, 255 } : Color{ 255, 64, 96 };
		circle.draw(fillColor);

		const auto buttonL = static_cast<ConfigurableButton>(kButtonLeftLaserL + i * 2);
		const auto buttonR = static_cast<ConfigurableButton>(kButtonLeftLaserL + i * 2 + 1);
		const Input& inputL = KeyConfig::GetConfigValue(targetConfigSet(), buttonL);
		const Input& inputR = KeyConfig::GetConfigValue(targetConfigSet(), buttonR);

		// どちらかのキーが重複していたら赤色で表示
		const Color textColor = (HasDuplicateKey(targetConfigSet(), buttonL) || HasDuplicateKey(targetConfigSet(), buttonR)) ? Palette::Red : Palette::White;
		m_font(U"{} {} {}"_fmt(InputToString(inputL), I18n::Get(I18n::Option::KeyConfigLaserKeySeparator), InputToString(inputR))).drawAt(Scaled(16), circle.center + Scaled(0, -50), textColor);
	}

	// Start/Back
	for (int32 i = 0; i < 2; ++i)
	{
		const Rect rect{ LeftMargin() + Scaled(260), Scaled(56 + 48 * i), Scaled(124), Scaled(32) };
		const auto matchCursor = static_cast<OptionKeyConfigCursor>(static_cast<int32>(OptionKeyConfigCursor::Start) + i);
		if (m_cursor == matchCursor)
		{
			const Color frameColor = m_state == OptionKeyConfigMenuState::SettingButton1 ? Palette::Yellow : Palette::Red;
			rect.drawFrame(Scaled(5), Scaled(5), frameColor);
		}
		rect.draw(Color{ 0, 24, 128 });

		const auto button = static_cast<ConfigurableButton>(kButtonStart + i);
		const Input& input = KeyConfig::GetConfigValue(targetConfigSet(), button);
		const StringView prefix1Sv = targetConfigSet() == KeyConfig::ConfigSet::kKeyboard1 ? U"*" : U"";
		const StringView prefix2Sv = i == 0 ? I18n::Get(I18n::Option::KeyConfigStart) : I18n::Get(I18n::Option::KeyConfigBack);
		const Color textColor = HasDuplicateKey(targetConfigSet(), button) ? Palette::Red : Palette::White;
		m_font(prefix1Sv + prefix2Sv + InputToString(input)).drawAt(Scaled(16), rect.center(), textColor);
	}

	// ConfigSet
	{
		const auto matchCursor = OptionKeyConfigCursor::ConfigSet;
		const Color fontColor = m_cursor == matchCursor ? Palette::Yellow : Palette::White;

		const StringView leftArrow = m_configSetMenu.isCursorMin() ? U" " : U"<";
		const StringView rightArrow = m_configSetMenu.isCursorMax() ? U" " : U">";

		String text;
		if (targetConfigSet() == KeyConfig::ConfigSet::kKeyboard1 || targetConfigSet() == KeyConfig::ConfigSet::kKeyboard2)
		{
			text = leftArrow + I18n::Get(I18n::Option::KeyConfigCategoryKeyboard, static_cast<int32>(targetConfigSet()) - KeyConfig::ConfigSet::kKeyboard1 + 1) + rightArrow;
		}
		else
		{
			text = leftArrow + I18n::Get(I18n::Option::KeyConfigCategoryGamepad, static_cast<int32>(targetConfigSet()) - KeyConfig::ConfigSet::kGamepad1 + 1) + rightArrow;
		}
		m_font(text).drawAt(Scaled(16), Point{ Scene::Center().x + Scaled(10), Scaled(410) }, fontColor);
	}
}
