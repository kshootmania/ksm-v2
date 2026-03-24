#include "HispeedSettingMenu.hpp"
#include "Input/PlatformKey.hpp"
#include "Input/KeyConfig.hpp"

namespace MusicGame
{
	namespace
	{
		constexpr int32 kHispeedMin = 25;
		constexpr int32 kHispeedMax = 2000;
		constexpr int32 kHispeedStep = 25;
		constexpr int32 kHispeedXModValueMin = 1; // x0.1
		constexpr int32 kHispeedXModValueMax = 99; // x9.9
		constexpr int32 kHispeedXModValueStep = 1;

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

		/// @brief 現在のハイスピード値に最も近いハイスピード値が得られるハイスピード設定値を計算(ハイスピードの種類変更時に前の種類での値に最も近い設定にするために使用)
		/// @param currentHispeed 現在のハイスピード値
		/// @param currentBPM 現在のBPM
		/// @param stdBPM O-mod用の基準BPM
		/// @param targetHispeedType 対象とするハイスピードの種類
		/// @return ハイスピード設定値
		/// @remarks 戻り値は範囲外の値を丸めたり25刻みにしたりしないので、使用側で適用すること
		int32 NearestHispeedSettingValue(int32 currentHispeed, double currentBPM, double stdBPM, HispeedType targetHispeedType)
		{
			switch (targetHispeedType)
			{
			case HispeedType::XMod:
				return MathUtils::RoundToInt(currentHispeed * 10 / Max(currentBPM, 1.0));
			case HispeedType::OMod:
				return MathUtils::RoundToInt(currentHispeed * stdBPM / Max(currentBPM, 1.0));
			case HispeedType::CMod:
				return currentHispeed;
			default:
				assert(false && "Unknown hispeed type");
				return 0;
			}
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
		m_typeMenu.setCursorToValue(hispeedSetting.type);
		refreshValueMenuConstraints();
		m_valueMenu.setCursor(hispeedSetting.value);
	}

	HispeedSettingMenu::HispeedSettingMenu(const Array<HispeedType>& availableHispeedTypes, const HispeedSetting& hispeedSetting, double stdBPM, double initialBPM)
		: m_typeMenu(
			availableHispeedTypes,
			LinearMenu::CreateInfoWithCursorMinMax{
				.cursorInputCreateInfo = {
					.type = CursorInput::Type::Horizontal,
					.buttonFlags = CursorButtonFlags::kArrowOrFX,
					.buttonIntervalSec = 0.12,
					.needStartButtonHoldForNonArrowKey = NeedStartButtonHoldForNonArrowKeyYN::Yes,
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
					.needStartButtonHoldForNonArrowKey = NeedStartButtonHoldForNonArrowKeyYN::Yes,
				},
				.cursorMin = 0, // このあとすぐrefreshValueMenuConstraintsで値が入るのでここでは両方0でOK
				.cursorMax = 0,
				.cyclic = IsCyclicMenuYN::No,
			})
		, m_stdBPM(stdBPM)
	{
		setHispeedSetting(hispeedSetting);

		// m_currentHispeedを初期化
		switch (hispeedSetting.type)
		{
		case HispeedType::XMod:
			m_currentHispeed = MathUtils::RoundToInt(hispeedSetting.value * initialBPM / 10.0);
			break;
		case HispeedType::OMod:
			m_currentHispeed = MathUtils::RoundToInt(hispeedSetting.value * initialBPM / m_stdBPM);
			break;
		case HispeedType::CMod:
			m_currentHispeed = hispeedSetting.value;
			break;
		default:
			assert(false && "Unknown hispeed type");
			m_currentHispeed = 0;
			break;
		}
	}

	bool HispeedSettingMenu::update(double currentBPM)
	{
		// Ctrlキー押下中は早送り機能と干渉するため、ハイスピードメニューの操作を無効化
		if (PlatformKey::KeyCommandControl.pressed())
		{
			return false;
		}

		m_typeMenu.update();
		m_valueMenu.update();

		if (m_typeMenu.deltaCursor() != 0)
		{
			refreshValueMenuConstraints();

			// 新しいハイスピードの種類で現在の値に最も近いハイスピード設定値を調べて設定
			const HispeedType hispeedType = m_typeMenu.cursorValue();
			const int32 nearestValue = NearestHispeedSettingValue(m_currentHispeed, currentBPM, m_stdBPM, hispeedType);
			m_valueMenu.setCursor(ApplyConstraints(nearestValue, hispeedType));
		}

		// 現在のハイスピード値を更新
		const HispeedSetting setting = hispeedSetting();
		switch (setting.type)
		{
		case HispeedType::XMod:
			m_currentHispeed = MathUtils::RoundToInt(setting.value * currentBPM / 10.0);
			break;
		case HispeedType::OMod:
			m_currentHispeed = MathUtils::RoundToInt(setting.value * currentBPM / m_stdBPM);
			break;
		case HispeedType::CMod:
			m_currentHispeed = setting.value;
			break;
		default:
			assert(false && "Unknown hispeed type");
			break;
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
