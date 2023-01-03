#include "hispeed_setting_menu.hpp"

namespace
{
	using MusicGame::HispeedType;
	using MusicGame::HispeedSetting;

	constexpr int32 kHispeedMin = 25;
	constexpr int32 kHispeedMax = 2000;
	constexpr int32 kHispeedDefault = 400;
	constexpr int32 kHispeedStep = 25;
	constexpr int32 kHispeedXModValueMin = 1; // x0.1
	constexpr int32 kHispeedXModValueMax = 99; // x9.9
	constexpr int32 kHispeedXModValueDefault = 10; // x1.0
	constexpr int32 kHispeedXModValueStep = 1;

	Array<HispeedType> LoadAvailableTypesFromConfigIni()
	{
		Array<HispeedType> availableTypes;
		availableTypes.reserve(static_cast<std::size_t>(HispeedType::EnumCount));

		if (ConfigIni::GetBool(ConfigIni::Key::kHispeedShowXMod, true))
		{
			availableTypes.push_back(HispeedType::XMod);
		}
		if (ConfigIni::GetBool(ConfigIni::Key::kHispeedShowOMod, true))
		{
			availableTypes.push_back(HispeedType::OMod);
		}
		if (ConfigIni::GetBool(ConfigIni::Key::kHispeedShowCMod, false)) // C-modのみデフォルトでは非表示のためfalse
		{
			availableTypes.push_back(HispeedType::CMod);
		}

		return availableTypes;
	}

	String HispeedSettingToString(const HispeedSetting& hispeedSetting)
	{
		switch (hispeedSetting.type)
		{
		case HispeedType::XMod:
			return U"x{:0>2}"_fmt(hispeedSetting.value);

		case HispeedType::OMod:
			return U"{}"_fmt(hispeedSetting.value);

		case HispeedType::CMod:
			return U"C{}"_fmt(hispeedSetting.value);

		default:
			assert(false && "Unknown hispeed type");
			return String{};
		}
	}

	HispeedSetting ParseHispeedSetting(StringView sv)
	{
		// OpenSiv3Dでゼロ始まりの整数がParseできることを念のため確認
		// (OpenSiv3Dのバージョン変更で挙動が変わった場合に検知できるよう残しておく)
		assert(ParseOr<int32>(U"05", 0) == 5);

		if (sv.length() <= 1U)
		{
			// 最低文字数の2文字に満たない場合はデフォルト値を返す
			return HispeedSetting{};
		}

		switch (sv[0])
		{
		case U'x':
			return HispeedSetting{
				.type = HispeedType::XMod,
				.value = Clamp(ParseOr<int32>(sv.substr(1U), 0), kHispeedXModValueMin, kHispeedXModValueMax),
			};

		case U'C':
			return HispeedSetting{
				.type = HispeedType::CMod,
				.value = Clamp(ParseOr<int32>(sv.substr(1U), 0), kHispeedMin, kHispeedMax),
			};

		default: // o-modは数字のみ
			return HispeedSetting{
				.type = HispeedType::OMod,
				.value = Clamp(ParseOr<int32>(sv, 0), kHispeedMin, kHispeedMax),
			};
		}
	}

	HispeedSetting LoadFromConfigIni()
	{
		return ParseHispeedSetting(ConfigIni::GetString(ConfigIni::Key::kHispeed));
	}

	void SaveToConfigIni(const HispeedSetting& hispeedSetting)
	{
		ConfigIni::SetString(ConfigIni::Key::kHispeed, HispeedSettingToString(hispeedSetting));
	}
}

void HispeedSettingMenu::refreshValueMenuConstraints()
{
	switch (m_typeMenu.cursor<HispeedType>())
	{
	case HispeedType::XMod:
		m_valueMenu.setCursorMinMax(kHispeedXModValueMin, kHispeedXModValueMax);
		m_valueMenu.setCursorStep(kHispeedXModValueStep);
		break;

	case HispeedType::OMod:
	case HispeedType::CMod:
		m_valueMenu.setCursorMinMax(kHispeedMin, kHispeedMax);
		m_valueMenu.setCursorStep(kHispeedStep);
		break;

	default:
		assert(false && "Unknown hispeed type");
		break;
	}
}

void HispeedSettingMenu::setHispeedSetting(const HispeedSetting& hispeedSetting)
{
	m_typeMenu.setCursor(hispeedSetting.type);
	refreshValueMenuConstraints();
	m_valueMenu.setCursor(hispeedSetting.value);
}

HispeedSettingMenu::HispeedSettingMenu()
	: m_typeMenu(
		LoadAvailableTypesFromConfigIni(),
		LinearMenu::CreateInfoWithCursorMinMax{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Horizontal,
				.buttonFlags = CursorButtonFlags::kArrowOrFX,
				.buttonIntervalSec = 0.12,
				.startRequiredForBTFXLaser = StartRequiredForBTFXLaserYN::Yes,
			},
			.cyclic = IsCyclicMenuYN::Yes,
		})
	, m_valueMenu(
		LinearMenu::CreateInfoWithCursorMinMax{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Vertical,
				.buttonFlags = CursorButtonFlags::kArrowOrLaserAll,
				.flipArrowKeyDirection = FlipArrowKeyDirectionYN::Yes, // 上向きで増加、下向きで減少なので、上下逆にする
				.buttonIntervalSec = 0.06,
				.startRequiredForBTFXLaser = StartRequiredForBTFXLaserYN::Yes,
			},
			.cursorMin = 0, // このあとすぐrefreshValueMenuConstraintsで値が入るのでここでは両方0でOK
			.cursorMax = 0,
			.cyclic = IsCyclicMenuYN::No,
		})
{
	loadFromConfigIni();
}

void HispeedSettingMenu::update()
{
	m_typeMenu.update();
	m_valueMenu.update();

	if (m_typeMenu.deltaCursor() != 0)
	{
		refreshValueMenuConstraints();
	}
}

void HispeedSettingMenu::loadFromConfigIni()
{
	setHispeedSetting(LoadFromConfigIni());
}

void HispeedSettingMenu::saveToConfigIni() const
{
	SaveToConfigIni(hispeedSetting());
}

HispeedSetting HispeedSettingMenu::hispeedSetting() const
{
	return HispeedSetting{
		.type = m_typeMenu.cursor<HispeedType>(),
		.value = m_valueMenu.cursor(),
	};
}
