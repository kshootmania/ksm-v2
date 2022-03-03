#pragma once

namespace ConfigIni
{
	namespace Key
	{
		// Option > Display / Sound Settings
		constexpr StringView kFullScreen = U"fullscr";
		constexpr StringView kWindowResolution = U"windowsize";
		constexpr StringView kFullScreenResolution = U"fullscrsize";
		constexpr StringView kLanguage = U"currentlang";
		constexpr StringView kTextureSize = U"lowquality";
		constexpr StringView kBGDisplayMode = U"background";
		constexpr StringView kAlwaysShowOtherFolders = U"showalldir";
		constexpr StringView kHideAllFolder = U"hide_itemall";
		constexpr StringView kMasterVolume = U"mastervol";
		constexpr StringView kVsync = U"vsync";

		// Option > Input / Judgment Settings
		constexpr StringView kJudgmentModeBT = U"bt_mode";
		constexpr StringView kJudgmentModeFX = U"fx_mode";
		constexpr StringView kJudgmentModeLaser = U"laser_mode";
		constexpr StringView kLaserInputType = U"laserinput";
		constexpr StringView kAssistTick = U"assisttick";
		constexpr StringView kDisableIME = U"disableime";
		constexpr StringView kTimingAdjust = U"inputdelay";
		constexpr StringView kLaserTimingAdjust = U"laserdelay";
		constexpr StringView kLaserMouseDirectionX = U"mouse_directionx";
		constexpr StringView kLaserMouseDirectionY = U"mouse_directiony";
		constexpr StringView kLaserSignalSensitivity = U"mouse_senstivity";
		constexpr StringView kSwapLaserLR = U"switchlaser";
		constexpr StringView kSelectKeyCloseFolder = U"closekey";
		constexpr StringView kUse3BTsPlusStartAsBack = U"esckey_bt3";

		// Option > Other Settings
		constexpr StringView kActiveHispeedTypes = U"hispeedtype";
		constexpr StringView kHideMouseCursor = U"hidemousecursor";
		constexpr StringView kUseNumpadAsArrowKeys = U"usenumpad";

		// Option > Key Config
		constexpr StringView kKeyConfigKeyboard1 = U"key";
		constexpr StringView kKeyConfigKeyboard2 = U"key2";
		constexpr StringView kKeyConfigGamepad1 = U"joy";
		constexpr StringView kKeyConfigGamepad2 = U"joy2";

		// Song Select / Game Play
		constexpr StringView kIRSignInCursor = U"ir_sign_in_cursor";
		constexpr StringView kCurrentPlayer = U"currentplayer";
		constexpr StringView kSelectDirectory = U"currentdir";
		constexpr StringView kSelectSongIndex = U"currentitem";
		constexpr StringView kSelectDifficulty = U"currentlevel";
		constexpr StringView kSelectSortType = U"mselview";
		constexpr StringView kHispeed = U"hispeed";
		constexpr StringView kEffRateType = U"effratetype";
		constexpr StringView kShowFastSlow = U"viewtiming";
		constexpr StringView kNoteSkin = U"noteskin";
		constexpr StringView kBGMovie = U"backgroundmovie";

		// Last launched version
		constexpr StringView kVersion = U"currentver";

		// Other advanced audio settings
		constexpr StringView kGlobalOffset = U"globaloffset";
		constexpr StringView kAudioFXDelay = U"soundfx_delay";
		constexpr StringView kVisualOffset = U"visual_offset";
		constexpr StringView kAutoPlaySE = U"auto_play_se";

		constexpr StringView kMuteAudioInInactiveWindow = U"automaticmute";

		constexpr StringView kExportPNG = U"output";
		constexpr StringView kExportPNGPath = U"output_path";
		constexpr StringView kExportPNGDownscale = U"output_downscale";

		constexpr StringView kEffRateZoom = U"effrate_zoom";
		constexpr StringView kEffRateShiftX = U"effrate_shiftx";
		constexpr StringView kEffRateShiftY = U"effrate_shifty";

		constexpr StringView kNetworkProxyHost = U"proxy_host";
		constexpr StringView kNetworkProxyPortNumber = U"proxy_port";
	}

	namespace Value
	{
		// Note: "enum class" is not used here to use implicit cast to int32

		namespace TextureSize
		{
			enum TextureSize : int32
			{
				kSmall = 0,
				kMedium,
				kLarge,
			};
		}

		namespace BGDisplayMode
		{
			enum BGDisplayMode : int32
			{
				kHide = 0,
				kShowNoAnim,
				kShowAnim,
			};
		}

		namespace JudgmentMode
		{
			enum JudgmentMode : int32
			{
				kOn = 0,
				kOff,
				kAuto,
				kHide,
			};
		}
	}

	void Load();

	// TODO: save function

	bool HasValue(StringView key);

	int32 GetInt(StringView key, int32 defaultValue = 0);

	double GetDouble(StringView key, double defaultValue = 0.0);

	StringView GetString(StringView key, StringView defaultValue = U"");

	// TODO: setter functions
}
