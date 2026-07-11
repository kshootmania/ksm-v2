#pragma once
#include <CoTaskLib.hpp>
#include "Common/CommonDefines.hpp"

#ifdef __APPLE__
#include <ksmplatform_macos/input_method.h>
#endif

class ILaserInputMethod;

using NeedStartButtonHoldForNonArrowKeyYN = YesNo<struct NeedStartButtonHoldForNonArrowKeyYN_tag>;

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

	void SetConfigValueByCommaSeparated(ConfigSet targetConfigSet, StringView configValue);

	void SetConfigValue(ConfigSet targetConfigSet, ConfigurableButton button, const Input& input);

	const Input& GetConfigValue(ConfigSet targetConfigSet, ConfigurableButton button);

	void SaveToConfigIni();

	// KeyConfigの入力状態を更新(毎フレーム呼び出す)
	void Update();

	bool Pressed(Button button);

	Optional<Button> LastPressedLaserButton(Button button1, Button button2);

	/// @brief レーザー入力からカーソルの移動量を取得
	/// @param laneIdx レーンのインデックス(0=左LASER, 1=右LASER)
	/// @param deltaTimeSec 前フレームからの経過時間(秒)
	/// @return カーソルのX方向移動量
	double LaserDeltaCursorX(int32 laneIdx, double deltaTimeSec);

	/// @brief レーザー入力がデジタル(キーボード)かどうかを判定
	/// @return デジタル入力の場合true、アナログ入力の場合false
	[[nodiscard]]
	bool IsLaserInputDigital();

	bool Down(Button button);

	/// @brief 仮想的にボタンを1フレームだけDown扱いにする
	void RequestVirtualDown(Button button);

	void ClearInput(Button button);

	Co::Task<void> WaitUntilDown(Button button);

	bool Up(Button button);

	template <class C>
	bool AnyButtonPressed(const C& buttons, NeedStartButtonHoldForNonArrowKeyYN needStartButtonHoldForNonArrowKey = NeedStartButtonHoldForNonArrowKeyYN::No)
	{
		const bool nonArrowKeyAccepted = !needStartButtonHoldForNonArrowKey || KeyConfig::Pressed(kButtonStart);
		for (const auto& button : buttons)
		{
			const bool accepted = nonArrowKeyAccepted || IsButtonArrowKey(button);
			if (accepted && KeyConfig::Pressed(button))
			{
				return true;
			}
		}
		return false;
	}

	template <class C>
	bool AnyButtonDown(const C& buttons, NeedStartButtonHoldForNonArrowKeyYN needStartButtonHoldForNonArrowKey = NeedStartButtonHoldForNonArrowKeyYN::No)
	{
		const bool nonArrowKeyAccepted = !needStartButtonHoldForNonArrowKey || KeyConfig::Pressed(kButtonStart); // Startボタン判定側は押しっぱなしかの判定なのでPressedで正しい
		for (const auto& button : buttons)
		{
			const bool accepted = nonArrowKeyAccepted || IsButtonArrowKey(button);
			if (accepted && KeyConfig::Down(button))
			{
				return true;
			}
		}
		return false;
	}
}
