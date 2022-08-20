#include "config_ini.hpp"
#include "ksm_ini_data.hpp"
#include "input/key_config.hpp"

namespace
{
	constexpr FilePathView kConfigIniFilePath = U"config.ini";

	KSMIniData s_configIniData;
}

void ConfigIni::Load()
{
	s_configIniData.load(kConfigIniFilePath);

	// 旧バージョンの"hispeedtype"の値を読み込む
	if (s_configIniData.hasValue(Key::kHispeedShownModsLegacy))
	{
		const String commaSeparatedMods = s_configIniData.getString(Key::kHispeedShownModsLegacy) + U',';

		// Note: 旧バージョンでは誤って"O"と"C"が逆に保存されていたため、正しく読み込むために意図的に入れ替えてある
		using StrPair = std::pair<StringView, StringView>;
		for (const auto& [key, modComma] : {
				StrPair{ Key::kHispeedShowXMod, U"x," },
				StrPair{ Key::kHispeedShowOMod, U"C,"/* ← 食い違っているが意図的 */ },
				StrPair{ Key::kHispeedShowCMod, U"O,"/* ← 食い違っているが意図的 */ } })
		{
			if (!s_configIniData.hasValue(key))
			{
				s_configIniData.setBool(key, commaSeparatedMods.includes(modComma));
			}
		}
	}

	// キーコンフィグの読み込み
	using KeyConfigTypes = std::initializer_list<std::tuple<StringView, KeyConfig::ConfigSet>>;
	for (const auto & [iniKey, targetConfigSet]
		: KeyConfigTypes{
			{ Key::kKeyConfigKeyboard1, KeyConfig::kKeyboard1 },
			{ Key::kKeyConfigKeyboard2, KeyConfig::kKeyboard2 },
			{ Key::kKeyConfigGamepad1,  KeyConfig::kGamepad1  },
			{ Key::kKeyConfigGamepad2,  KeyConfig::kGamepad2  } })
	{
		const StringView defaultValue = KeyConfig::kDefaultConfigValues[targetConfigSet];
		const StringView configValue = GetString(iniKey, defaultValue);
		KeyConfig::SetConfigValue(targetConfigSet, configValue);
	}
}

void ConfigIni::Save()
{
	s_configIniData.save(kConfigIniFilePath);
}

bool ConfigIni::HasValue(StringView key)
{
	return s_configIniData.hasValue(key);
}

bool ConfigIni::GetBool(StringView key, bool defaultValue)
{
	return s_configIniData.getBool(key, defaultValue);
}

int32 ConfigIni::GetInt(StringView key, int32 defaultValue)
{
	return s_configIniData.getInt(key, defaultValue);
}

double ConfigIni::GetDouble(StringView key, double defaultValue)
{
	return s_configIniData.getDouble(key, defaultValue);
}

StringView ConfigIni::GetString(StringView key, StringView defaultValue)
{
	return s_configIniData.getString(key, defaultValue);
}

void ConfigIni::SetBool(StringView key, bool value)
{
	s_configIniData.setBool(key, value);
}

void ConfigIni::SetInt(StringView key, int32 value)
{
	s_configIniData.setInt(key, value);
}

void ConfigIni::SetDouble(StringView key, double value)
{
	s_configIniData.setDouble(key, value);
}

void ConfigIni::SetString(StringView key, StringView value)
{
	s_configIniData.setString(key, value);
}
