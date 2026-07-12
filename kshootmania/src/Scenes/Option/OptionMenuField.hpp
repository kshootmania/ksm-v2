#pragma once
#include "UI/LinearMenu.hpp"

class OptionMenuField
{
private:
	const String m_configIniKey;
	const String m_name;

	bool m_isEnum;

	// 数値オプション用
	const String m_suffixStr;
	const String m_suffixStrZero;
	const String m_suffixStrPositive;
	const String m_suffixStrNegative;

	// 選択肢オプション用
	const Array<std::pair<String, String>> m_valueDisplayNamePairs;

	const std::function<void()> m_onChangeCallback;

	LinearMenu m_menu;

public:
	struct CreateInfo
	{
		String configIniKey;
		String name;

		// 数値オプション用
		int32 valueMin = 0;
		int32 valueMax = 0;
		int32 valueDefault = 0;
		int32 valueStep = 0;
		String suffixStr;
		String suffixStrZero;
		String suffixStrPositive;
		String suffixStrNegative;

		// 選択肢オプション用
		Array<std::pair<String, String>> valueDisplayNamePairs;

		std::function<void()> onChangeCallback = nullptr;

		static CreateInfo Enum(StringView name, StringView configIniKey, const Array<String>& valueDisplayNames);

		static CreateInfo Enum(StringView name, StringView configIniKey, const Array<StringView>& valueDisplayNames);

		static CreateInfo Enum(StringView name, StringView configIniKey, const Array<std::pair<String, String>>& valueDisplayNamePairs);

		static CreateInfo Enum(StringView name, StringView configIniKey, const Array<std::pair<int, String>>& valueDisplayNamePairs);

		static CreateInfo Enum(StringView name, StringView configIniKey, const Array<std::pair<double, String>>& valueDisplayNamePairs);

		static CreateInfo Int(StringView name, StringView configIniKey, int32 valueMin = std::numeric_limits<int32>::min(), int32 valueMax = std::numeric_limits<int32>::max(), int32 valueDefault = 0, StringView suffixStr = U"", int32 valueStep = 1);

		CreateInfo& setAdditionalSuffixes(StringView zero, StringView positive, StringView negative)&;

		CreateInfo&& setAdditionalSuffixes(StringView zero, StringView positive, StringView negative)&&;

		/// @brief config.iniに値が無い場合のデフォルト値を設定(Enumの場合は選択肢のインデックス)
		CreateInfo& setDefaultValue(int32 value)&;

		CreateInfo&& setDefaultValue(int32 value)&&;

		CreateInfo& setOnChangeCallback(std::function<void()> callback)&;

		CreateInfo&& setOnChangeCallback(std::function<void()> callback)&&;
	};

	explicit OptionMenuField(const CreateInfo& createInfo);

	/// @brief 更新処理
	/// @return 値が変更された場合true
	bool update();

	/// @brief 外部からの値変更量を加えた更新処理(クリック操作による値変更用)
	/// @param externalValueDelta 外部からの値変更量
	/// @return 値が変更された場合true
	bool update(int32 externalValueDelta);

	// 注意: これらの列挙子の値はNocoUI側のvalueArrowIndexにも使用されるので順番を入れ替えないこと
	enum ArrowType
	{
		kArrowTypeLeft = 0,
		kArrowTypeRight,
		kArrowTypeLeftRight,
		kArrowTypeNone,

		kArrowTypeEnumCount,
	};

	[[nodiscard]]
	const String& name() const;

	[[nodiscard]]
	String valueDisplayString() const;

	[[nodiscard]]
	ArrowType arrowType() const;
};
