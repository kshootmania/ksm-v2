#pragma once
#include "UI/LinearMenu.hpp"
#include "OptionMenuField.hpp"

class OptionMenu
{
public:
	struct SettingItemParams
	{
		String name;
		String value;
		bool selected;
		int32 valueArrowIndex;
	};

private:
	LinearMenu m_menu;

	Array<OptionMenuField> m_fields;

public:
	explicit OptionMenu(const Array<OptionMenuField::CreateInfo>& fieldCreateInfos);

	/// @brief メニューを更新する
	/// @return カーソルまたは値に変更があった場合true
	[[nodiscard]]
	bool update();

	/// @brief 設定項目のパラメータ一覧を取得する
	[[nodiscard]]
	Array<SettingItemParams> getSettingItemParamsList() const;

	[[nodiscard]]
	int32 cursor() const;

	void setCursor(int32 value);
};
