#pragma once

namespace EnumUtils
{
	template <typename T>
	[[nodiscard]]
	String EnumToString(T value) requires std::is_enum_v<T>
	{
		return Unicode::FromUTF8(magic_enum::enum_name(value));
	}

	template <typename T>
	[[nodiscard]]
	T StringToEnum(StringView value, T defaultValue) requires std::is_enum_v<T>
	{
		const auto u8Value = value.toUTF8();
		if (magic_enum::enum_contains<T>(u8Value))
		{
			return magic_enum::enum_cast<T>(u8Value).value();
		}
		return defaultValue;
	}

	template <typename T>
	[[nodiscard]]
	Optional<T> StringToEnumOpt(StringView value) requires std::is_enum_v<T>
	{
		const auto u8Value = value.toUTF8();
		if (magic_enum::enum_contains<T>(u8Value))
		{
			return magic_enum::enum_cast<T>(u8Value).value();
		}
		return none;
	}

	template <typename T>
	[[nodiscard]]
	bool EnumContains(StringView value) requires std::is_enum_v<T>
	{
		return magic_enum::enum_contains<T>(value.toUTF8());
	}

	template <typename T>
	[[nodiscard]]
	Array<String> EnumNames() requires std::is_enum_v<T>
	{
		Array<String> result;
		result.reserve(magic_enum::enum_count<T>());
		for (const auto& name : magic_enum::enum_names<T>())
		{
			result.push_back(Unicode::FromUTF8(name));
		}
		return result;
	}
}
