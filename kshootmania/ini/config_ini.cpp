#include "config_ini.hpp"
#include "ksm_ini_data.hpp"
#include "key_config.hpp"

namespace
{
	constexpr FilePathView kConfigIniFilePath = U"config.ini";

	KSMIniData s_configIniData;
}

void ConfigIni::Load()
{
	s_configIniData.load(kConfigIniFilePath);

	// Set key configurations
	using KeyConfigTypes = std::initializer_list<std::tuple<StringView, KeyConfig::ConfigSet>>;
	for (auto [iniKey, targetConfigSet]
		: KeyConfigTypes{
			{ Key::kKeyConfigKeyboard1, KeyConfig::kKeyboard1 },
			{ Key::kKeyConfigKeyboard2, KeyConfig::kKeyboard2 },
			{ Key::kKeyConfigGamepad1,  KeyConfig::kGamepad1  },
			{ Key::kKeyConfigGamepad2,  KeyConfig::kGamepad2  }})
	{
		const StringView defaultValue = KeyConfig::kDefaultConfigValues[targetConfigSet];
		const StringView configValue = GetString(iniKey, defaultValue);
		KeyConfig::SetConfigValue(targetConfigSet, configValue);
	}
}

bool ConfigIni::HasValue(StringView key)
{
	return s_configIniData.hasValue(key);
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
