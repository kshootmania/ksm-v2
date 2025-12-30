#pragma once
#include <unordered_map>
#include "Input/KeyConfig.hpp"
#include "Input/Cursor/CursorInput.hpp"
#include "UI/LinearMenu.hpp"

enum class OptionKeyConfigCursor
{
	Start,
	Back,
	BT_A,
	BT_B,
	BT_C,
	BT_D,
	FX_L,
	FX_R,
	FX_LR,
	Laser_L,
	Laser_R,
	ConfigSet,
};

enum class OptionKeyConfigMenuState
{
	None,
	SettingButton1,
	SettingButton2,
};

class OptionKeyConfigMenu
{
private:
	CursorInput m_horizontalCursorInput;
	CursorInput m_verticalCursorInput;
	OptionKeyConfigCursor m_cursor = OptionKeyConfigCursor::BT_A;
	OptionKeyConfigMenuState m_state = OptionKeyConfigMenuState::None;
	LinearMenu m_configSetMenu;

#ifdef __APPLE__
	// macOSプラットフォーム特有キーの前フレーム状態
	std::unordered_map<int, bool> m_platformKeyWasPressed;
#endif

	[[nodiscard]]
	KeyConfig::ConfigSet targetConfigSet() const;

	void setInput(const Input& input);

	void updateNoneState();

	void updateSettingButtonState();

public:
	OptionKeyConfigMenu();

	void update();

	void updateUI(noco::Canvas* pCanvas) const;

	[[nodiscard]]
	bool isButtonEditingState() const
	{
		return m_state != OptionKeyConfigMenuState::None;
	}
};
