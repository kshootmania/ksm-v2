#include "OptionMenuField.hpp"
#include "Ini/ConfigIni.hpp"

namespace
{
	template <typename T>
	Array<std::pair<String, String>> ConvertValueDisplayNamePairs(const Array<std::pair<T, String>>& valueDisplayNamePairs)
	{
		return valueDisplayNamePairs.map([](const std::pair<T, String>& pair) {
			return std::pair<String, String>{ Format(pair.first), pair.second };
		});
	}

	Array<std::pair<String, String>> ConvertValueDisplayNamePairs(const Array<String>& valueDisplayNames)
	{
		Array<std::pair<String, String>> pairs;
		for (std::size_t i = 0; i < valueDisplayNames.size(); ++i)
		{
			pairs.emplace_back(Format(i), valueDisplayNames[i]);
		}
		return pairs;
	}

	Array<std::pair<String, String>> ConvertValueDisplayNamePairs(const Array<StringView>& valueDisplayNames)
	{
		Array<std::pair<String, String>> pairs;
		for (std::size_t i = 0; i < valueDisplayNames.size(); ++i)
		{
			pairs.emplace_back(Format(i), valueDisplayNames[i]);
		}
		return pairs;
	}

	LinearMenu::CreateInfoWithCursorMinMax MakeMenuInt(int32 valueMin, int32 valueMax, int32 valueDefault, int32 valueStep)
	{
		return LinearMenu::CreateInfoWithCursorMinMax{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Horizontal,
				.buttonFlags = CursorButtonFlags::kArrowOrLaser,
				.buttonIntervalSec = 0.1,
				.buttonIntervalSecFirst = 0.5,
			},
			.cursorMin = valueMin,
			.cursorMax = valueMax,
			.cursorStep = valueStep,
			.defaultCursor = valueDefault,
		};
	}

	LinearMenu::CreateInfoWithCursorMinMax MakeMenuEnum(int32 enumCount, int32 valueDefault)
	{
		return LinearMenu::CreateInfoWithCursorMinMax{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Horizontal,
				.buttonFlags = CursorButtonFlags::kArrowOrLaser,
				.buttonIntervalSec = 0.1,
				.buttonIntervalSecFirst = 0.5,
			},
			.cursorMin = 0,
			.cursorMax = Max(enumCount - 1, 0),
			.defaultCursor = valueDefault,
		};
	}

	OptionMenuField::ArrowType GetMenuFieldValueArrowType(const LinearMenu& menu)
	{
		const bool isMin = menu.isCursorMin();
		const bool isMax = menu.isCursorMax();

		if (isMin && isMax)
		{
			return OptionMenuField::kArrowTypeNone;
		}

		if (isMin)
		{
			return OptionMenuField::kArrowTypeRight;
		}

		if (isMax)
		{
			return OptionMenuField::kArrowTypeLeft;
		}

		return OptionMenuField::kArrowTypeLeftRight;
	}

	int32 FindDefaultCursor(StringView configIniKey, bool isEnum, const Array<std::pair<String, String>>& valueDisplayNamePairs, int32 cursorDefault)
	{
		if (ConfigIni::HasValue(configIniKey))
		{
			if (isEnum)
			{
				const StringView value = ConfigIni::GetString(configIniKey);
				for (std::size_t i = 0; i < valueDisplayNamePairs.size(); ++i)
				{
					if (valueDisplayNamePairs[i].first == value)
					{
						return static_cast<int32>(i);
					}
				}
			}
			else
			{
				return ConfigIni::GetInt(configIniKey, cursorDefault);
			}
		}

		return cursorDefault;
	}
}

OptionMenuField::CreateInfo OptionMenuField::CreateInfo::Enum(StringView name, StringView configIniKey, const Array<String>& valueDisplayNames)
{
	return Enum(name, configIniKey, ConvertValueDisplayNamePairs(valueDisplayNames));
}

OptionMenuField::CreateInfo OptionMenuField::CreateInfo::Enum(StringView name, StringView configIniKey, const Array<StringView>& valueDisplayNames)
{
	return Enum(name, configIniKey, ConvertValueDisplayNamePairs(valueDisplayNames));
}

OptionMenuField::CreateInfo OptionMenuField::CreateInfo::Enum(StringView name, StringView configIniKey, const Array<std::pair<String, String>>& valueDisplayNamePairs)
{
	return CreateInfo{
		.configIniKey = String{ configIniKey },
		.name = String{ name },
		.valueDisplayNamePairs = valueDisplayNamePairs,
	};
}

OptionMenuField::CreateInfo OptionMenuField::CreateInfo::Enum(StringView name, StringView configIniKey, const Array<std::pair<int, String>>& valueDisplayNamePairs)
{
	return Enum(name, configIniKey, ConvertValueDisplayNamePairs(valueDisplayNamePairs));
}

OptionMenuField::CreateInfo OptionMenuField::CreateInfo::Enum(StringView name, StringView configIniKey, const Array<std::pair<double, String>>& valueDisplayNamePairs)
{
	return Enum(name, configIniKey, ConvertValueDisplayNamePairs(valueDisplayNamePairs));
}

OptionMenuField::CreateInfo OptionMenuField::CreateInfo::Int(StringView name, StringView configIniKey, int32 valueMin, int32 valueMax, int32 valueDefault, StringView suffixStr, int32 valueStep)
{
	if (valueStep == 0)
	{
		// CreateInfo having zero valueStep is misrecognized as an enum field, so throw an exception.
		throw Error(U"CreateInfo::Int(): valueStep must not be zero!");
	}

	return OptionMenuField::CreateInfo{
		.configIniKey = String{ configIniKey },
		.name = String{ name },
		.valueMin = valueMin,
		.valueMax = valueMax,
		.valueDefault = valueDefault,
		.valueStep = valueStep,
		.suffixStr = String{ suffixStr },
	};
}

OptionMenuField::CreateInfo& OptionMenuField::CreateInfo::setAdditionalSuffixes(StringView zero, StringView positive, StringView negative)&
{
	this->suffixStrZero = String{ zero };
	this->suffixStrPositive = String{ positive };
	this->suffixStrNegative = String{ negative };
	return *this;
}

OptionMenuField::CreateInfo&& OptionMenuField::CreateInfo::setAdditionalSuffixes(StringView zero, StringView positive, StringView negative)&&
{
	this->suffixStrZero = String{ zero };
	this->suffixStrPositive = String{ positive };
	this->suffixStrNegative = String{ negative };
	return std::move(*this);
}

OptionMenuField::CreateInfo& OptionMenuField::CreateInfo::setOnChangeCallback(std::function<void()> callback)&
{
	onChangeCallback = std::move(callback);
	return *this;
}

OptionMenuField::CreateInfo&& OptionMenuField::CreateInfo::setOnChangeCallback(std::function<void()> callback)&&
{
	onChangeCallback = std::move(callback);
	return std::move(*this);
}

OptionMenuField::OptionMenuField(const CreateInfo& createInfo)
	: m_configIniKey(createInfo.configIniKey)
	, m_name(createInfo.name)
	, m_isEnum(createInfo.valueStep == 0)
	, m_suffixStr(createInfo.suffixStr)
	, m_suffixStrZero(createInfo.suffixStrZero)
	, m_suffixStrPositive(createInfo.suffixStrPositive)
	, m_suffixStrNegative(createInfo.suffixStrNegative)
	, m_valueDisplayNamePairs(createInfo.valueDisplayNamePairs)
	, m_onChangeCallback(createInfo.onChangeCallback)
	, m_menu(createInfo.valueStep == 0
		? MakeMenuEnum(static_cast<int32>(createInfo.valueDisplayNamePairs.size()), FindDefaultCursor(m_configIniKey, m_isEnum, m_valueDisplayNamePairs, 0))
		: MakeMenuInt(createInfo.valueMin, createInfo.valueMax, FindDefaultCursor(m_configIniKey, m_isEnum, m_valueDisplayNamePairs, createInfo.valueDefault), createInfo.valueStep))
{
}

bool OptionMenuField::update()
{
	const int32 cursorPrev = m_menu.cursor();

	m_menu.update();

	// Update config.ini value if cursor is changed
	const int32 cursor = m_menu.cursor();
	if (cursor != cursorPrev)
	{
		if (m_isEnum)
		{
			const int32 enumCount = static_cast<int32>(m_valueDisplayNamePairs.size());
			if (0 <= cursor && cursor < enumCount)
			{
				ConfigIni::SetString(m_configIniKey, m_valueDisplayNamePairs[cursor].first);
			}
			else
			{
				Logger << U"[ksm warning] OptionMenuField::update(): Option enum value index is out of range! (index={}, min=0, max={})"_fmt(cursor, enumCount - 1);
			}
		}
		else
		{
			ConfigIni::SetInt(m_configIniKey, cursor);
		}

		if (m_onChangeCallback != nullptr)
		{
			m_onChangeCallback();
		}

		return true;
	}

	return false;
}

String OptionMenuField::valueDisplayString() const
{
	const int32 cursor = m_menu.cursor();

	if (m_isEnum)
	{
		const int32 enumCount = static_cast<int32>(m_valueDisplayNamePairs.size());
		if (cursor < 0 || enumCount <= cursor)
		{
			return U"";
		}
		return String{ m_valueDisplayNamePairs[cursor].second };
	}
	else
	{
		if (!m_suffixStrZero.isEmpty())
		{
			String additionalSuffix;
			if (cursor == 0)
			{
				additionalSuffix = m_suffixStrZero;
			}
			else if (cursor > 0)
			{
				additionalSuffix = m_suffixStrPositive;
			}
			else
			{
				additionalSuffix = m_suffixStrNegative;
			}

			if (cursor > 0)
			{
				return U"+" + Format(cursor) + m_suffixStr + additionalSuffix;
			}
			else
			{
				return Format(cursor) + m_suffixStr + additionalSuffix;
			}
		}
		else
		{
			return Format(cursor) + m_suffixStr;
		}
	}
}

const String& OptionMenuField::name() const
{
	return m_name;
}

OptionMenuField::ArrowType OptionMenuField::arrowType() const
{
	return GetMenuFieldValueArrowType(m_menu);
}
