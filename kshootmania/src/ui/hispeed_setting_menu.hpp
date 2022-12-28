#pragma once
#include "linear_menu.hpp"

/// @brief ハイスピードの種類
enum class HispeedType : int32
{
	XMod, // x-mod (倍率指定)
	OMod, // o-mod (最頻BPM基準での指定)
	CMod, // C-mod (定数指定)
	// KSMv1(HSP版)のa-mod、m-modは廃止
	EnumCount,
};

/// @brief ハイスピード設定
struct HispeedSetting
{
	/// @brief ハイスピードの種類
	HispeedType type = HispeedType::XMod;

	/// @brief ハイスピード値
	/// @remarks typeがXModの場合は倍率を10倍した値(1以上99以下の整数)。それ以外の場合はBPMに倍率をかけた後のハイスピード値(25以上2000以下)
	int32 value = 10;

	/// @brief config.iniのハイスピード値の文字列に変換
	/// @return ハイスピード値の文字列(例: "x05", "750", "C750")
	String ToString() const;

	/// @brief config.iniのハイスピード値の文字列をパース
	/// @param sv ハイスピード値の文字列(例: "x05", "750", "C750")
	/// @return HispeedSetting構造体
	static HispeedSetting Parse(StringView sv);
};

/// @brief ハイスピード設定のメニュー
class HispeedSettingMenu
{
private:
	ArrayWithLinearMenu<HispeedType> m_typeMenu;
	LinearMenu m_valueMenu;

	void refreshValueMenuConstraints();

	HispeedSetting hispeedSetting() const;

	void setHispeedSetting(const HispeedSetting& hispeedSetting);

public:
	/// @brief コンストラクタ
	/// @remarks 内部でConfigIniを参照するため、ConfigIniが初期化済みである必要がある
	HispeedSettingMenu();

	/// @brief 毎フレームの更新
	void update();

	/// @brief ConfigIniから読み込んでメニューに反映
	/// @remarks ハイスピード設定の表示/非表示設定に更新があっても反映されない点に注意
	void loadFromConfigIni();

	/// @brief ConfigIniへメニューの状態を保存
	void saveToConfigIni() const;
};
