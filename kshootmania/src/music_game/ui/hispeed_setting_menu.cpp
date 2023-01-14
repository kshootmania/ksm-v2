#include "hispeed_setting_menu.hpp"

namespace MusicGame
{
	namespace
	{
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

			// 1つも有効でない場合はデフォルトのもの(x-mod、o-mod)を追加
			if (availableTypes.empty())
			{
				availableTypes.push_back(HispeedType::XMod);
				availableTypes.push_back(HispeedType::OMod);
			}

			return availableTypes;
		}

		int32 CursorMin(HispeedType hispeedType)
		{
			switch (hispeedType)
			{
			case HispeedType::XMod:
				return kHispeedXModValueMin;

			case HispeedType::OMod:
			case HispeedType::CMod:
				return kHispeedMin;

			default:
				assert(false && "Unknown hispeed type");
				return kHispeedMin;
			}
		}

		int32 CursorMax(HispeedType hispeedType)
		{
			switch (hispeedType)
			{
			case HispeedType::XMod:
				return kHispeedXModValueMax;

			case HispeedType::OMod:
			case HispeedType::CMod:
				return kHispeedMax;

			default:
				assert(false && "Unknown hispeed type");
				return kHispeedMax;
			}
		}

		int32 CursorStep(HispeedType hispeedType)
		{
			switch (hispeedType)
			{
			case HispeedType::XMod:
				return kHispeedXModValueStep;

			case HispeedType::OMod:
			case HispeedType::CMod:
				return kHispeedStep;

			default:
				assert(false && "Unknown hispeed type");
				return kHispeedStep;
			}
		}

		/// @brief ハイスピード設定値の制約を適用
		/// @param value 元の値
		/// @param hispeedType ハイスピードの種類
		/// @return ハイスピード設定値
		int32 ApplyConstraints(int32 value, HispeedType hispeedType)
		{
			const int32 cursorMin = CursorMin(hispeedType);
			const int32 cursorMax = CursorMax(hispeedType);
			const int32 cursorStep = CursorStep(hispeedType);
			return Clamp(MathUtils::RoundToInt(static_cast<double>(value) / cursorStep) * cursorStep, cursorMin, cursorMax);
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
			if (sv.length() <= 1U)
			{
				// 最低文字数の2文字に満たない場合はデフォルト値を返す
				return HispeedSetting{};
			}

			switch (sv[0])
			{
			case U'x':
			{
				// 0始まりだと8進数扱いにされて"09"がパース失敗になってしまうため、先頭のゼロを除去している
				const int32 value = ParseOr<int32>(sv.substr(sv.starts_with(U"x0") ? 2U : 1U), 0);
				return HispeedSetting{
					.type = HispeedType::XMod,
					.value = ApplyConstraints(value, HispeedType::XMod),
				};
			}

			case U'C':
				return HispeedSetting{
					.type = HispeedType::CMod,
					.value = ApplyConstraints(ParseOr<int32>(sv.substr(1U), 0), HispeedType::CMod),
				};

			default: // o-modは数字のみ
				return HispeedSetting{
					.type = HispeedType::OMod,
					.value = ApplyConstraints(ParseOr<int32>(sv, 0), HispeedType::OMod),
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
		const HispeedType hispeedType = m_typeMenu.cursorValue();
		m_valueMenu.setCursorMinMax(CursorMin(hispeedType), CursorMax(hispeedType));
		m_valueMenu.setCursorStep(CursorStep(hispeedType));
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

	bool HispeedSettingMenu::update(const Scroll::HighwayScroll& highwayScroll)
	{
		m_typeMenu.update();
		m_valueMenu.update();

		if (m_typeMenu.deltaCursor() != 0)
		{
			refreshValueMenuConstraints();

			// 新しいハイスピードの種類で現在の値に最も近いハイスピード設定値を調べて設定
			const HispeedType hispeedType = m_typeMenu.cursorValue();
			const int32 nearestValue = highwayScroll.nearestHispeedSettingValue(hispeedType);
			m_valueMenu.setCursor(ApplyConstraints(nearestValue, hispeedType));
		}

		// 値に変更があったか返す
		return m_valueMenu.deltaCursor() != 0 || m_typeMenu.deltaCursor() != 0;
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
			.type = m_typeMenu.cursorValue(),
			.value = m_valueMenu.cursor(),
		};
	}
}
