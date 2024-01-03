#pragma once

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
	KeyConfig::ConfigSet m_targetConfigSet = KeyConfig::ConfigSet::kKeyboard1;
	const Font m_font = AssetManagement::SystemFont();

	const TiledTexture m_fxLRTexture;

	void setInput(const Input& input);

	void updateNoneState();

	void updateSettingButtonState();

public:
	OptionKeyConfigMenu();

	void update();

	void draw() const;
};
