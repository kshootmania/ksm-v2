#include "BTOptionPanel.hpp"
#include "Common/CommonDefines.hpp"
#include "Common/MathUtils.hpp"
#include "I18n/I18n.hpp"
#include "Ini/ConfigIni.hpp"
#include "RuntimeConfig.hpp"
#include "Input/Cursor/CursorInput.hpp"
#include "MusicGame/HispeedUtils.hpp"

namespace
{
	using MusicGame::HispeedType;

	// BT-Aメニューの項目
	enum class BTAMenuItem : int32
	{
		kJudgmentBT = 0,
		kJudgmentFX,
		kJudgmentLaser,

		kCount,
	};

	// BT-Bメニューの項目
	enum class BTBMenuItem : int32
	{
		kEffRate = 0,
		kTurn,
		kPlaybackSpeed,

		kCount,
	};

	// BT-Cメニューの項目
	enum class BTCMenuItem : int32
	{
		kAssistTick = 0,
		kAutoSync,
		kFastSlow,
		kNoteSkin,
		kMovie,

		kCount,
	};

	// ハイスピード値の範囲
	constexpr int32 kHispeedXModMin = 1; // x-modの最小値(x0.1)
	constexpr int32 kHispeedXModMax = 99; // x-modの最大値(x9.9)
	constexpr int32 kHispeedOCModMin = 25; // o/c-modの最小値
	constexpr int32 kHispeedOCModMax = 2000; // o/c-modの最大値
	constexpr int32 kHispeedOCModStep = 25; // o/c-modの刻み幅

	// 再生速度の範囲
	constexpr int32 kPlaybackSpeedMin = 10; // 10%
	constexpr int32 kPlaybackSpeedMax = 300; // 300%
	constexpr int32 kPlaybackSpeedStep = 5; // 5%刻み

	int32 CursorMin(HispeedType hispeedType)
	{
		switch (hispeedType)
		{
		case HispeedType::XMod:
			return kHispeedXModMin;
		case HispeedType::OMod:
		case HispeedType::CMod:
			return kHispeedOCModMin;
		default:
			return kHispeedOCModMin;
		}
	}

	int32 CursorMax(HispeedType hispeedType)
	{
		switch (hispeedType)
		{
		case HispeedType::XMod:
			return kHispeedXModMax;
		case HispeedType::OMod:
		case HispeedType::CMod:
			return kHispeedOCModMax;
		default:
			return kHispeedOCModMax;
		}
	}

	int32 CursorStep(HispeedType hispeedType)
	{
		switch (hispeedType)
		{
		case HispeedType::XMod:
			return 1;
		case HispeedType::OMod:
		case HispeedType::CMod:
			return kHispeedOCModStep;
		default:
			return kHispeedOCModStep;
		}
	}

	StringView JudgmentModeToI18nKey(ConfigIni::Value::JudgmentMode::JudgmentMode mode)
	{
		switch (mode)
		{
		case ConfigIni::Value::JudgmentMode::kOn: return I18n::Get(I18n::Select::JudgmentModeOn);
		case ConfigIni::Value::JudgmentMode::kOff: return I18n::Get(I18n::Select::JudgmentModeOff);
		case ConfigIni::Value::JudgmentMode::kAuto: return I18n::Get(I18n::Select::JudgmentModeAuto);
		case ConfigIni::Value::JudgmentMode::kHide: return I18n::Get(I18n::Select::JudgmentModeHide);
		default: return U"";
		}
	}

	StringView GaugeTypeToI18nKey(GaugeType gauge)
	{
		switch (gauge)
		{
		case GaugeType::kEasyGauge: return I18n::Get(I18n::Select::EffRateEasy);
		case GaugeType::kNormalGauge: return I18n::Get(I18n::Select::EffRateNormal);
		case GaugeType::kHardGauge: return I18n::Get(I18n::Select::EffRateHard);
		default: return U"";
		}
	}

	StringView TurnModeToI18nKey(TurnMode turn)
	{
		switch (turn)
		{
		case TurnMode::kNormal: return I18n::Get(I18n::Select::TurnNormal);
		case TurnMode::kMirror: return I18n::Get(I18n::Select::TurnMirror);
		case TurnMode::kRandom: return I18n::Get(I18n::Select::TurnRandom);
		default: return U"";
		}
	}

	StringView AssistTickModeToI18nKey(AssistTickMode assistTick)
	{
		switch (assistTick)
		{
		case AssistTickMode::kOff: return I18n::Get(I18n::Select::AssistTickOff);
		case AssistTickMode::kOn: return I18n::Get(I18n::Select::AssistTickOn);
		default: return U"";
		}
	}

	StringView AutoSyncModeToI18nKey(AutoSyncMode autoSync)
	{
		switch (autoSync)
		{
		case AutoSyncMode::kOff: return I18n::Get(I18n::Select::AutoSyncOff);
		case AutoSyncMode::kLow: return I18n::Get(I18n::Select::AutoSyncOnLow);
		case AutoSyncMode::kMid: return I18n::Get(I18n::Select::AutoSyncOnMid);
		case AutoSyncMode::kHigh: return I18n::Get(I18n::Select::AutoSyncOnHigh);
		default: return U"";
		}
	}

	StringView FastSlowModeToI18nKey(FastSlowMode display)
	{
		switch (display)
		{
		case FastSlowMode::kHide: return I18n::Get(I18n::Select::FastSlowHide);
		case FastSlowMode::kShow: return I18n::Get(I18n::Select::FastSlowShow);
		default: return U"";
		}
	}

	StringView NoteSkinTypeToI18nKey(NoteSkinType skin)
	{
		switch (skin)
		{
		case NoteSkinType::kDefault: return I18n::Get(I18n::Select::NoteSkinDefault);
		case NoteSkinType::kNote: return I18n::Get(I18n::Select::NoteSkinNote);
		default: return U"";
		}
	}

	StringView MovieModeToI18nKey(MovieMode display)
	{
		switch (display)
		{
		case MovieMode::kOff: return I18n::Get(I18n::Select::MovieOff);
		case MovieMode::kOn: return I18n::Get(I18n::Select::MovieOn);
		default: return U"";
		}
	}

	// 再生速度のカーソル値をもとに表示文字列を取得
	String PlaybackSpeedToString(int32 cursor)
	{
		return U" {}% "_fmt(cursor);
	}

	String FormatMenuLine(StringView label, StringView value, bool isSelected, int32 currentCursor, int32 minCursor, int32 maxCursor)
	{
		String line{ label };

		// 左端の場合は<を表示しない
		const bool showLeft = isSelected && currentCursor > minCursor;
		// 右端の場合は>を表示しない
		const bool showRight = isSelected && currentCursor < maxCursor;

		line += showLeft ? U"<" : U" ";
		line += value;
		if (showRight)
		{
			line += U">";
		}
		return line;
	}
}

BTOptionPanel::BTOptionPanel(std::shared_ptr<noco::Canvas> canvas)
	: m_canvas(canvas)
	, m_btAMenu(LinearMenu::CreateInfoWithEnumCount{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Vertical,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		},
		.enumCount = static_cast<int32>(BTAMenuItem::kCount),
		.cyclic = IsCyclicMenuYN::No,
	})
	, m_btBMenu(LinearMenu::CreateInfoWithEnumCount{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Vertical,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		},
		.enumCount = static_cast<int32>(BTBMenuItem::kCount),
		.cyclic = IsCyclicMenuYN::No,
	})
	, m_btCMenu(LinearMenu::CreateInfoWithEnumCount{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Vertical,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		},
		.enumCount = static_cast<int32>(BTCMenuItem::kCount),
		.cyclic = IsCyclicMenuYN::No,
	})

	// BT-Aメニューの値変更用
	, m_judgmentModeBT(LinearMenu::CreateInfoWithEnumCount{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Horizontal,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		},
		.enumCount = 4, // JudgmentModeの値の数(On, Off, Auto, Hide)
		.cyclic = IsCyclicMenuYN::No,
	})
	, m_judgmentModeFX(LinearMenu::CreateInfoWithEnumCount{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Horizontal,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		},
		.enumCount = 4, // JudgmentModeの値の数(On, Off, Auto, Hide)
		.cyclic = IsCyclicMenuYN::No,
	})
	, m_judgmentModeLaser(LinearMenu::CreateInfoWithEnumCount{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Horizontal,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		},
		.enumCount = 4, // JudgmentModeの値の数(On, Off, Auto, Hide)
		.cyclic = IsCyclicMenuYN::No,
	})

	// BT-Bメニューの値変更用
	, m_gaugeType(LinearMenu::CreateInfoWithCursorMinMax{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Horizontal,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		},
		.cursorMin = static_cast<int32>(GaugeType::kEasyGauge),
		.cursorMax = static_cast<int32>(GaugeType::kHardGauge),
		.cyclic = IsCyclicMenuYN::No,
	})
	, m_turnMode(LinearMenu::CreateInfoWithEnumCount{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Horizontal,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		},
		.enumCount = static_cast<int32>(3),
		.cyclic = IsCyclicMenuYN::No,
	})
	, m_playbackSpeed(LinearMenu::CreateInfoWithCursorMinMax{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Horizontal,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
			.buttonIntervalSec = 0.1,
			.buttonIntervalSecFirst = 0.4,
		},
		.cursorMin = kPlaybackSpeedMin,
		.cursorMax = kPlaybackSpeedMax,
		.cursorStep = kPlaybackSpeedStep,
		.cyclic = IsCyclicMenuYN::No,
	})

	// BT-Cメニューの値変更用
	, m_assistTick(LinearMenu::CreateInfoWithEnumCount{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Horizontal,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		},
		.enumCount = static_cast<int32>(AssistTickMode::kCount),
		.cyclic = IsCyclicMenuYN::No,
	})
	, m_autoSync(LinearMenu::CreateInfoWithEnumCount{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Horizontal,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		},
		.enumCount = static_cast<int32>(AutoSyncMode::kCount),
		.cyclic = IsCyclicMenuYN::No,
	})
	, m_fastSlow(LinearMenu::CreateInfoWithEnumCount{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Horizontal,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		},
		.enumCount = static_cast<int32>(FastSlowMode::kCount),
		.cyclic = IsCyclicMenuYN::No,
	})
	, m_noteSkin(LinearMenu::CreateInfoWithEnumCount{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Horizontal,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		},
		.enumCount = static_cast<int32>(NoteSkinType::kCount),
		.cyclic = IsCyclicMenuYN::No,
	})
	, m_movie(LinearMenu::CreateInfoWithEnumCount{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Horizontal,
			.buttonFlags = CursorButtonFlags::kArrowOrLaser,
		},
		.enumCount = static_cast<int32>(MovieMode::kCount),
		.cyclic = IsCyclicMenuYN::No,
	})

	// BT-Dメニュー(ハイスピード)用
	, m_hispeedTypeMenu(
		ConfigIni::LoadAvailableHispeedTypes(),
		LinearMenu::CreateInfoWithCursorMinMax{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Horizontal,
				.buttonFlags = CursorButtonFlags::kArrowOrLaser,
				.buttonIntervalSec = 0.12,
			},
			.cyclic = IsCyclicMenuYN::No,
		})
	, m_hispeedValueMenu(
		LinearMenu::CreateInfoWithCursorMinMax{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Vertical,
				.buttonFlags = CursorButtonFlags::kArrowOrLaser,
				.flipArrowKeyDirection = FlipArrowKeyDirectionYN::Yes, // 上向きで増加、下向きで減少
				.buttonIntervalSec = 0.06,
			},
			.cursorMin = 0,
			.cursorMax = 0,
			.cyclic = IsCyclicMenuYN::No,
		})
{
	// ConfigIniから設定値を読み込み
	loadFromConfigIni();
}

Optional<Button> BTOptionPanel::getCurrentSingleBTButton() const
{
	// Shift押下中はBTメニューを開かない(アルファベットジャンプを優先)
	if (KeyShift.pressed())
	{
		return none;
	}

	int32 pressedCount = 0;
	Optional<Button> pressedButton;

	for (Button button = kButtonBT_A; button <= kButtonBT_D; ++button)
	{
		if (KeyConfig::Pressed(button))
		{
			++pressedCount;
			pressedButton = button;
		}
	}

	// 単独押しの場合のみ返す
	if (pressedCount == 1)
	{
		return pressedButton;
	}

	return none;
}

void BTOptionPanel::refreshHispeedValueMenuConstraints()
{
	const HispeedType hispeedType = m_hispeedTypeMenu.cursorValue();
	m_hispeedValueMenu.setCursorMinMax(CursorMin(hispeedType), CursorMax(hispeedType));
	m_hispeedValueMenu.setCursorStep(CursorStep(hispeedType));
}

String BTOptionPanel::generateBTAMenuText() const
{
	const auto judgmentModeBT = m_judgmentModeBT.cursorAs<ConfigIni::Value::JudgmentMode::JudgmentMode>();
	const auto judgmentModeFX = m_judgmentModeFX.cursorAs<ConfigIni::Value::JudgmentMode::JudgmentMode>();
	const auto judgmentModeLaser = m_judgmentModeLaser.cursorAs<ConfigIni::Value::JudgmentMode::JudgmentMode>();

	const auto currentItem = m_btAMenu.cursorAs<BTAMenuItem>();

	String text{ I18n::Get(I18n::Select::Judgment) };
	text += U"\n";
	text += FormatMenuLine(I18n::Get(I18n::Select::JudgmentBT), JudgmentModeToI18nKey(judgmentModeBT), currentItem == BTAMenuItem::kJudgmentBT, m_judgmentModeBT.cursor(), 0, 3);
	text += U"\n";
	text += FormatMenuLine(I18n::Get(I18n::Select::JudgmentFX), JudgmentModeToI18nKey(judgmentModeFX), currentItem == BTAMenuItem::kJudgmentFX, m_judgmentModeFX.cursor(), 0, 3);
	text += U"\n";
	text += FormatMenuLine(I18n::Get(I18n::Select::JudgmentLaser), JudgmentModeToI18nKey(judgmentModeLaser), currentItem == BTAMenuItem::kJudgmentLaser, m_judgmentModeLaser.cursor(), 0, 3);

	return text;
}

String BTOptionPanel::generateBTBMenuText() const
{
	const auto gaugeType = m_gaugeType.cursorAs<GaugeType>();
	const auto turnMode = m_turnMode.cursorAs<TurnMode>();
	const int32 playbackSpeedCursor = m_playbackSpeed.cursor();

	const auto currentItem = m_btBMenu.cursorAs<BTBMenuItem>();

	String text = U"";
	text += FormatMenuLine(I18n::Get(I18n::Select::EffRate), GaugeTypeToI18nKey(gaugeType), currentItem == BTBMenuItem::kEffRate, m_gaugeType.cursor(), static_cast<int32>(GaugeType::kEasyGauge), static_cast<int32>(GaugeType::kHardGauge));
	text += U"\n";
	text += FormatMenuLine(I18n::Get(I18n::Select::Turn), TurnModeToI18nKey(turnMode), currentItem == BTBMenuItem::kTurn, m_turnMode.cursor(), 0, 2);
	text += U"\n";
	text += FormatMenuLine(I18n::Get(I18n::Select::PlaybackSpeed), PlaybackSpeedToString(playbackSpeedCursor), currentItem == BTBMenuItem::kPlaybackSpeed, playbackSpeedCursor, kPlaybackSpeedMin, kPlaybackSpeedMax);

	return text;
}

String BTOptionPanel::generateBTCMenuText() const
{
	const auto assistTick = m_assistTick.cursorAs<AssistTickMode>();
	const auto autoSync = m_autoSync.cursorAs<AutoSyncMode>();
	const auto fastSlow = m_fastSlow.cursorAs<FastSlowMode>();
	const auto noteSkin = m_noteSkin.cursorAs<NoteSkinType>();
	const auto movie = m_movie.cursorAs<MovieMode>();

	const auto currentItem = m_btCMenu.cursorAs<BTCMenuItem>();

	String text = U"";
	text += FormatMenuLine(I18n::Get(I18n::Select::AssistTick), AssistTickModeToI18nKey(assistTick), currentItem == BTCMenuItem::kAssistTick, m_assistTick.cursor(), 0, static_cast<int32>(AssistTickMode::kCount) - 1);
	text += U"\n";
	text += FormatMenuLine(I18n::Get(I18n::Select::AutoSync), AutoSyncModeToI18nKey(autoSync), currentItem == BTCMenuItem::kAutoSync, m_autoSync.cursor(), 0, static_cast<int32>(AutoSyncMode::kCount) - 1);
	text += U"\n";
	text += FormatMenuLine(I18n::Get(I18n::Select::FastSlow), FastSlowModeToI18nKey(fastSlow), currentItem == BTCMenuItem::kFastSlow, m_fastSlow.cursor(), 0, static_cast<int32>(FastSlowMode::kCount) - 1);
	text += U"\n";
	text += FormatMenuLine(I18n::Get(I18n::Select::NoteSkin), NoteSkinTypeToI18nKey(noteSkin), currentItem == BTCMenuItem::kNoteSkin, m_noteSkin.cursor(), 0, static_cast<int32>(NoteSkinType::kCount) - 1);
	text += U"\n";
	text += FormatMenuLine(I18n::Get(I18n::Select::Movie), MovieModeToI18nKey(movie), currentItem == BTCMenuItem::kMovie, m_movie.cursor(), 0, static_cast<int32>(MovieMode::kCount) - 1);

	return text;
}

String BTOptionPanel::generateBTDMenuText() const
{
	const auto hispeedType = m_hispeedTypeMenu.cursorValue();
	const int32 hispeedValue = m_hispeedValueMenu.cursor();

	String text{ I18n::Get(I18n::Select::Hispeed) };
	text += U"\n";
	text += U"          ";
	text += MusicGame::HispeedUtils::ToDisplayString(MusicGame::HispeedSetting{ .type = hispeedType, .value = hispeedValue });

	return text;
}

bool BTOptionPanel::update(double currentChartStdBPM)
{
	const auto currentButton = getCurrentSingleBTButton();

	// ハイスコア再読み込みが必要な変更があったか
	bool needsHighScoreReload = false;

	// いずれかのBTボタンが単独で押されている場合
	if (currentButton)
	{
		m_isVisible = true;

		// 対応するパネルを表示
		m_canvas->setParamValue(U"overlay_btOptionPanelVisible_A", currentButton == kButtonBT_A);
		m_canvas->setParamValue(U"overlay_btOptionPanelVisible_B", currentButton == kButtonBT_B);
		m_canvas->setParamValue(U"overlay_btOptionPanelVisible_C", currentButton == kButtonBT_C);
		m_canvas->setParamValue(U"overlay_btOptionPanelVisible_D", currentButton == kButtonBT_D);

		// 各メニューの更新
		if (currentButton == kButtonBT_A)
		{
			m_btAMenu.update();

			// 現在選択されている項目の値変更用LinearMenuを更新
			const auto currentItem = m_btAMenu.cursorAs<BTAMenuItem>();
			bool valueChanged = false;

			if (currentItem == BTAMenuItem::kJudgmentBT)
			{
				m_judgmentModeBT.update();
				valueChanged = m_judgmentModeBT.deltaCursor() != 0;
			}
			else if (currentItem == BTAMenuItem::kJudgmentFX)
			{
				m_judgmentModeFX.update();
				valueChanged = m_judgmentModeFX.deltaCursor() != 0;
			}
			else if (currentItem == BTAMenuItem::kJudgmentLaser)
			{
				m_judgmentModeLaser.update();
				valueChanged = m_judgmentModeLaser.deltaCursor() != 0;
			}

			if (valueChanged)
			{
				saveToConfigIni();
				needsHighScoreReload = true;
			}

			m_canvas->setParamValue(U"overlay_btOptionPanelText_A", generateBTAMenuText());
		}
		else if (currentButton == kButtonBT_B)
		{
			m_btBMenu.update();

			// 現在選択されている項目の値変更用LinearMenuを更新
			const auto currentItem = m_btBMenu.cursorAs<BTBMenuItem>();
			bool valueChanged = false;

			if (currentItem == BTBMenuItem::kEffRate)
			{
				m_gaugeType.update();
				valueChanged = m_gaugeType.deltaCursor() != 0;
				if (valueChanged)
				{
					RuntimeConfig::SetGaugeType(m_gaugeType.cursorAs<GaugeType>());
				}
			}
			else if (currentItem == BTBMenuItem::kTurn)
			{
				m_turnMode.update();
				valueChanged = m_turnMode.deltaCursor() != 0;
				if (valueChanged)
				{
					RuntimeConfig::SetTurnMode(m_turnMode.cursorAs<TurnMode>());
				}
			}
			else if (currentItem == BTBMenuItem::kPlaybackSpeed)
			{
				m_playbackSpeed.update();
				valueChanged = m_playbackSpeed.deltaCursor() != 0;
				if (valueChanged)
				{
					RuntimeConfig::SetPlaybackSpeed(m_playbackSpeed.cursor() / 100.0);
				}
			}

			if (valueChanged)
			{
				saveToConfigIni();
				needsHighScoreReload = true;
			}

			m_canvas->setParamValue(U"overlay_btOptionPanelText_B", generateBTBMenuText());
		}
		else if (currentButton == kButtonBT_C)
		{
			m_btCMenu.update();

			// 現在選択されている項目の値変更用LinearMenuを更新
			const auto currentItem = m_btCMenu.cursorAs<BTCMenuItem>();
			bool valueChanged = false;

			if (currentItem == BTCMenuItem::kAssistTick)
			{
				m_assistTick.update();
				valueChanged = m_assistTick.deltaCursor() != 0;
			}
			else if (currentItem == BTCMenuItem::kAutoSync)
			{
				m_autoSync.update();
				valueChanged = m_autoSync.deltaCursor() != 0;
			}
			else if (currentItem == BTCMenuItem::kFastSlow)
			{
				m_fastSlow.update();
				valueChanged = m_fastSlow.deltaCursor() != 0;
			}
			else if (currentItem == BTCMenuItem::kNoteSkin)
			{
				m_noteSkin.update();
				valueChanged = m_noteSkin.deltaCursor() != 0;
			}
			else if (currentItem == BTCMenuItem::kMovie)
			{
				m_movie.update();
				valueChanged = m_movie.deltaCursor() != 0;
			}

			if (valueChanged)
			{
				saveToConfigIni();
			}

			m_canvas->setParamValue(U"overlay_btOptionPanelText_C", generateBTCMenuText());
		}
		else if (currentButton == kButtonBT_D)
		{
			m_hispeedTypeMenu.update();
			m_hispeedValueMenu.update();

			bool valueChanged = false;

			// 種類が変更された場合、値の範囲を更新し、現在の値に最も近い値に設定
			if (m_hispeedTypeMenu.deltaCursor() != 0)
			{
				double bpm = currentChartStdBPM;
				if (bpm <= 0.0)
				{
					Logger << U"[ksm error] Invalid BPM value ({}) for hispeed type change. Using default BPM ({})."_fmt(bpm, kDefaultBPM);
					bpm = kDefaultBPM;
				}

				// 変更前の実際のハイスピード値を計算
				const HispeedType oldType = m_hispeedTypeMenu.atCyclic(m_hispeedTypeMenu.cursor() - m_hispeedTypeMenu.deltaCursor());
				const int32 oldValue = m_hispeedValueMenu.cursor();
				double actualHispeed;

				if (oldType == HispeedType::XMod)
				{
					actualHispeed = bpm * oldValue / 10.0;
				}
				else
				{
					actualHispeed = static_cast<double>(oldValue);
				}

				// 新しい種類での値を計算
				const HispeedType newType = m_hispeedTypeMenu.cursorValue();
				int32 newValue;

				if (newType == HispeedType::XMod)
				{
					newValue = static_cast<int32>(Math::Round(actualHispeed / bpm * 10.0));
					newValue = Clamp(newValue, kHispeedXModMin, kHispeedXModMax);
				}
				else
				{
					// o-mod, c-mod: 25刻みに丸める
					newValue = static_cast<int32>(Math::Round(actualHispeed / 25.0)) * 25;
					newValue = Clamp(newValue, kHispeedOCModMin, kHispeedOCModMax);
				}

				refreshHispeedValueMenuConstraints();
				m_hispeedValueMenu.setCursor(newValue);

				valueChanged = true;
			}
			else if (m_hispeedValueMenu.deltaCursor() != 0)
			{
				valueChanged = true;
			}

			if (valueChanged)
			{
				saveToConfigIni();
			}

			m_canvas->setParamValue(U"overlay_btOptionPanelText_D", generateBTDMenuText());
		}
	}
	else
	{
		m_isVisible = false;

		// BTボタンが押されていない場合はすべて非表示
		m_canvas->setParamValue(U"overlay_btOptionPanelVisible_A", false);
		m_canvas->setParamValue(U"overlay_btOptionPanelVisible_B", false);
		m_canvas->setParamValue(U"overlay_btOptionPanelVisible_C", false);
		m_canvas->setParamValue(U"overlay_btOptionPanelVisible_D", false);
	}

	return needsHighScoreReload;
}

bool BTOptionPanel::isVisible() const
{
	return m_isVisible;
}

void BTOptionPanel::hide()
{
	m_isVisible = false;
	m_canvas->setParamValue(U"overlay_btOptionPanelVisible_A", false);
	m_canvas->setParamValue(U"overlay_btOptionPanelVisible_B", false);
	m_canvas->setParamValue(U"overlay_btOptionPanelVisible_C", false);
	m_canvas->setParamValue(U"overlay_btOptionPanelVisible_D", false);
}

void BTOptionPanel::loadFromConfigIni()
{

	// BT-Aメニューの設定(config.iniには保存しない)
	m_judgmentModeBT.setCursor(static_cast<int32>(RuntimeConfig::GetJudgmentPlayModeBT()));
	m_judgmentModeFX.setCursor(static_cast<int32>(RuntimeConfig::GetJudgmentPlayModeFX()));
	m_judgmentModeLaser.setCursor(static_cast<int32>(RuntimeConfig::GetJudgmentPlayModeLaser()));

	// BT-Bメニューの設定(config.iniには保存しない)
	m_gaugeType.setCursor(static_cast<int32>(RuntimeConfig::GetGaugeType()));
	m_turnMode.setCursor(static_cast<int32>(RuntimeConfig::GetTurnMode()));
	m_playbackSpeed.setCursor(static_cast<int32>(Round(RuntimeConfig::GetPlaybackSpeed() * 100.0)));

	// BT-Cメニューの設定
	m_assistTick.setCursor(ConfigIni::GetInt(ConfigIni::Key::kAssistTick, static_cast<int32>(AssistTickMode::kOff)));

	m_autoSync.setCursor(ConfigIni::GetInt(ConfigIni::Key::kAutoSync, static_cast<int32>(AutoSyncMode::kOff)));
	m_fastSlow.setCursor(ConfigIni::GetInt(ConfigIni::Key::kShowFastSlow, static_cast<int32>(FastSlowMode::kHide)));

	// noteskinは文字列として保存される
	const StringView noteSkinStr = ConfigIni::GetString(ConfigIni::Key::kNoteSkin, U"default");
	if (noteSkinStr == U"note")
	{
		m_noteSkin.setCursor(static_cast<int32>(NoteSkinType::kNote));
	}
	else
	{
		m_noteSkin.setCursor(static_cast<int32>(NoteSkinType::kDefault));
	}

	m_movie.setCursor(ConfigIni::GetInt(ConfigIni::Key::kBGMovie, static_cast<int32>(MovieMode::kOn)));

	// BT-Dメニュー(ハイスピード)の設定
	const StringView hispeedStr = ConfigIni::GetString(ConfigIni::Key::kHispeed, U"x10");
	HispeedType loadedType = HispeedType::XMod;
	int32 loadedValue = 10;

	const MusicGame::HispeedSetting hispeedSetting = MusicGame::HispeedUtils::FromConfigStringValue(hispeedStr);
	loadedType = hispeedSetting.type;
	loadedValue = hispeedSetting.value;

	m_hispeedTypeMenu.setCursorToValue(loadedType);

	refreshHispeedValueMenuConstraints();
	m_hispeedValueMenu.setCursor(loadedValue);
}

void BTOptionPanel::saveToConfigIni()
{

	// BT-Aメニューの設定(config.iniには保存しない)
	RuntimeConfig::SetJudgmentPlayModeBT(m_judgmentModeBT.cursorAs<JudgmentPlayMode>());
	RuntimeConfig::SetJudgmentPlayModeFX(m_judgmentModeFX.cursorAs<JudgmentPlayMode>());
	RuntimeConfig::SetJudgmentPlayModeLaser(m_judgmentModeLaser.cursorAs<JudgmentPlayMode>());

	// BT-Bメニューの設定(config.iniには保存しない)
	RuntimeConfig::SetGaugeType(m_gaugeType.cursorAs<GaugeType>());
	RuntimeConfig::SetTurnMode(m_turnMode.cursorAs<TurnMode>());
	RuntimeConfig::SetPlaybackSpeed(m_playbackSpeed.cursor() / 100.0);

	// BT-Cメニューの設定
	ConfigIni::SetInt(ConfigIni::Key::kAssistTick, m_assistTick.cursor());

	ConfigIni::SetInt(ConfigIni::Key::kAutoSync, m_autoSync.cursor());
	ConfigIni::SetInt(ConfigIni::Key::kShowFastSlow, m_fastSlow.cursor());

	// noteskinは文字列として保存
	if (m_noteSkin.cursorAs<NoteSkinType>() == NoteSkinType::kNote)
	{
		ConfigIni::SetString(ConfigIni::Key::kNoteSkin, U"note");
	}
	else
	{
		ConfigIni::SetString(ConfigIni::Key::kNoteSkin, U"default");
	}

	ConfigIni::SetInt(ConfigIni::Key::kBGMovie, m_movie.cursor());

	// BT-Dメニュー(ハイスピード)の設定
	const HispeedType hispeedType = m_hispeedTypeMenu.cursorValue();
	const int32 hispeedValue = m_hispeedValueMenu.cursor();
	ConfigIni::SetString(ConfigIni::Key::kHispeed, MusicGame::HispeedUtils::ToConfigStringValue(MusicGame::HispeedSetting{ .type = hispeedType, .value = hispeedValue }));

	// ConfigIniをファイルに書き込み
	ConfigIni::Save();
}
