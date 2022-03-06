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

	// Convert legacy "hispeedtype" value
	if (s_configIniData.hasValue(Key::kHispeedShownModsLegacy))
	{
		const String commaSeparatedMods = s_configIniData.getString(Key::kHispeedShownModsLegacy) + U',';

		// Note: The "O" and "C" are *intentionally* swapped to simulate mistakes in the old version...
		using StrPair = std::pair<StringView, StringView>;
		for (const auto& [key, modComma] : {
				StrPair{ Key::kHispeedShowXMod, U"x," },
				StrPair{ Key::kHispeedShowOMod, U"C,"/* <- Intentional swap!! */ },
				StrPair{ Key::kHispeedShowCMod, U"O,"/* <- Intentional swap!! */ } })
		{
			if (!s_configIniData.hasValue(key))
			{
				s_configIniData.setBool(key, commaSeparatedMods.includes(modComma));
			}
		}
	}

	// Set key configurations
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
