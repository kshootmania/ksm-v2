#include "config_ini.hpp"
#include "ksm_ini_data.hpp"
#include "input/key_config.hpp"

namespace
{
	constexpr FilePathView kConfigIniFilePath = U"config.ini";

	constexpr StringView kDefaultConfigIni = UR"([config]
; キーコンフィグ
key=83,68,75,76,29,28,81,87,79,80,32,13,27,122
key2=72,74,70,71,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
joy=-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
joy2=-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1

; ハイスピード設定
hispeed=x10

; フルスクリーンの有効/無効 (1:有効 0:無効)
fullscr=0
; ウィンドウのサイズ(px,px)
windowsize=800,600
; フルスクリーン時の解像度(px,px)
fullscrsize=1280,720

; フレームのスキップ数(0～9)
frameskip=0

; ASSIST TICK機能の有効/無効 (1:有効 0:無効)
assisttick=0

playtype=2

highquality=0
lowquality=1

globaloffset=0

judgedifficulty=5

bt_mode=0
fx_mode=0
laser_mode=0

laserinput=0

background=2
backgroundmovie=1

noteskin=default

viewtiming=0

currentplayer=PLAYER

; outputの値を1にするとoutput_pathで指定したディレクトリに連番PNGが30fpsで書き出されます
; output_pathで指定するパスの終端には\を入れ忘れないよう注意してください
output=0
output_path=c:\output\
output_downscale=1

disableime=3
hidemousecursor=0

mastervol=100

mouse_directionx=1
mouse_directiony=1
mouse_sensitivity=4

switchlaser=0

currentlang=Japanese
currentdir=
currentitem=0
currentlevel=0

showalldir=1
closekey=0

currentver=170

mselview=directory
editor_disabledd=0
editor_assisttick=0
editor_reducespacing=0
editor_zoomlanex=0
editor_increasewinsize=1
editor_measurepercolumn=4
laserdelay=0
usenumpad=0
hispeedtype=x,C
editor_linebrightness=0
inputdelay=0
editor_loopplayback=1

vsync=0
esckey_bt3=1
twitter_token=
twitter_token_secret1=
twitter_token_secret2=
twitter_token_secret3=
hideitem_all=0
ir_sign_in_cursor=0
hispeedtype_o=1
hispeedtype_c=0
hispeedtype_x=0
)";

	KSMIniData s_configIniData;
}

void ConfigIni::Load()
{
	// config.iniが存在しない場合はデフォルトの内容でファイル作成
	if (!FileSystem::IsFile(kConfigIniFilePath))
	{
		TextWriter textWriter(kConfigIniFilePath, TextEncoding::UTF8_NO_BOM);
		textWriter.write(kDefaultConfigIni);
	}

	// ファイルを読み込む
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
		KeyConfig::SetConfigValueByCommaSeparated(targetConfigSet, configValue);
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
