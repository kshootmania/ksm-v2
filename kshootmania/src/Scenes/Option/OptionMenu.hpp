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

	/// @brief マウス操作の入力内容
	struct MouseInput
	{
		/// @brief クリックされた項目のインデックス(クリックされていない場合はnone)
		Optional<int32> clickedItemIdx;

		/// @brief 左右端のクリックまたは長押しによる値変更量
		int32 valueDelta = 0;
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

	/// @brief マウス操作の入力内容を加えてメニューを更新する
	/// @param mouseInput マウス操作の入力内容
	/// @return カーソルまたは値に変更があった場合true
	[[nodiscard]]
	bool update(const MouseInput& mouseInput);

	/// @brief 設定項目のパラメータ一覧を取得する
	[[nodiscard]]
	Array<SettingItemParams> getSettingItemParamsList() const;

	[[nodiscard]]
	int32 cursor() const;

	void setCursor(int32 value);
};
