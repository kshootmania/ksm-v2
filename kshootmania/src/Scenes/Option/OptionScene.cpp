#include "OptionScene.hpp"
#include "Common/FrameRateLimit.hpp"
#include "Common/IMEUtils.hpp"
#include "RuntimeConfig.hpp"
#include "Scenes/Title/TitleScene.hpp"
#include "Input/InputUtils.hpp"

// TODO: TextureIdxまわりどうにかする

namespace
{
	constexpr Duration kFadeDuration = 0.6s;

	FilePath GetOptionSceneUIFilePath()
	{
		return FsUtils::GetResourcePath(U"ui/scene/option.noco");
	}

	std::shared_ptr<noco::Canvas> LoadOptionSceneCanvas()
	{
		const FilePath uiFilePath = GetOptionSceneUIFilePath();
		const auto canvas = noco::Canvas::LoadFromFile(uiFilePath);
		if (!canvas)
		{
			throw Error{ U"Failed to load '{}'"_fmt(uiFilePath) };
		}
		return canvas;
	}

	constexpr int32 kMasterVolumeMin = 0;
	constexpr int32 kMasterVolumeMax = 100;
	constexpr int32 kMasterVolumeDefault = 100;
	constexpr int32 kTimingAdjustMin = -10000;
	constexpr int32 kTimingAdjustMax = 10000;
	constexpr int32 kTimingAdjustDefault = 0;
	constexpr int32 kLaserInputSensitivityMin = -10000;
	constexpr int32 kLaserInputSensitivityMax = 10000;
	constexpr int32 kLaserInputSensitivityDefault = 50;
}

std::unique_ptr<std::array<OptionMenu, OptionScene::kOptionMenuTypeEnumCount>> OptionScene::makeOptionMenus()
{
	using StrPair = std::pair<String, String>;
	using IntStrPair = std::pair<int32, String>;
	using DoubleStrPair = std::pair<double, String>;
	using CreateInfo = OptionMenuField::CreateInfo;

	Array<StrPair> availableLanguageStrPairs;
	for (const auto& language : I18n::GetAvailableLanguageList())
	{
		availableLanguageStrPairs.emplace_back(language, language);
	}

	return std::make_unique<std::array<OptionMenu, kOptionMenuTypeEnumCount>>(std::array<OptionMenu, kOptionMenuTypeEnumCount>{
		OptionMenu({
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemScreenMode), ConfigIni::Key::kFullScreen, Array<StringView>{
				I18n::Get(I18n::Option::FullscreenOff),
				I18n::Get(I18n::Option::FullscreenOn),
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemScreenResolutionWindow), ConfigIni::Key::kWindowResolution, Array<StrPair>{
				StrPair{ U"320,240", U"320x240" },
				StrPair{ U"480,320", U"480x320" },
				StrPair{ U"480,360", U"480x360" },
				StrPair{ U"512,384", U"512x384" },
				StrPair{ U"580,435", U"580x435" },
				StrPair{ U"640,480", U"640x480" },
				StrPair{ U"800,600", U"800x600" },
				StrPair{ U"1024,768", U"1024x768" },
				StrPair{ U"1280,720", U"1280x720" },
				StrPair{ U"1280,1024", U"1280x1024" },
				StrPair{ U"1366,768", U"1366x768" },
				StrPair{ U"1920,1080", U"1920x1080" },
				StrPair{ U"1920,1200", U"1920x1200" },
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemScreenResolutionFullscreen), ConfigIni::Key::kFullScreenResolution, Array<StrPair>{
				StrPair{ U"640,480", U"640x480" },
				StrPair{ U"800,600", U"800x600" },
				StrPair{ U"1024,600", U"1024x600" },
				StrPair{ U"1024,768", U"1024x768" },
				StrPair{ U"1280,720", U"1280x720" },
				StrPair{ U"1280,800", U"1280x800" },
				StrPair{ U"1280,1024", U"1280x1024" },
				StrPair{ U"1366,768", U"1366x768" },
				StrPair{ U"1920,1080", U"1920x1080" },
				StrPair{ U"1920,1200", U"1920x1200" },
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemDisplayLanguage), ConfigIni::Key::kLanguage, availableLanguageStrPairs)
				.setOnChangeCallback([this]
				{
					I18n::LoadLanguage(ConfigIni::GetString(ConfigIni::Key::kLanguage));
					refreshOptionMenusForLanguageChange();
				}),
			/*CreateInfo::Enum(ConfigIni::Key::kTextureSize, Array<IntStrPair>{
				IntStrPair{ ConfigIni::Value::TextureSize::kSmall, I18n::Get(I18n::Option::TextureSizeSmall) },
				IntStrPair{ ConfigIni::Value::TextureSize::kMedium, I18n::Get(I18n::Option::TextureSizeMedium) },
				IntStrPair{ ConfigIni::Value::TextureSize::kLarge, I18n::Get(I18n::Option::TextureSizeLarge) },
			}),*/
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemGameplayBG), ConfigIni::Key::kBGDisplayMode, Array<IntStrPair>{
				IntStrPair{ ConfigIni::Value::BGDisplayMode::kHide, I18n::Get(I18n::Option::BGDisplayModeHide) },
				IntStrPair{ ConfigIni::Value::BGDisplayMode::kShowNoLayer, I18n::Get(I18n::Option::BGDisplayModeShowNoLayer) },
				IntStrPair{ ConfigIni::Value::BGDisplayMode::kShowLayer, I18n::Get(I18n::Option::BGDisplayModeShowLayer) },
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemAlwaysShowOtherFolders), ConfigIni::Key::kAlwaysShowOtherFolders, Array<StringView>{
				I18n::Get(I18n::Option::AlwaysShowOtherFoldersOff),
				I18n::Get(I18n::Option::AlwaysShowOtherFoldersOn),
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemHideAllFolder), ConfigIni::Key::kHideAllFolder, Array<StringView>{
				I18n::Get(I18n::Option::HideAllFolderOff),
				I18n::Get(I18n::Option::HideAllFolderOn),
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemAssistTick), ConfigIni::Key::kAssistTick, Array<StringView>{
				I18n::Get(I18n::Option::AssistTickOff),
				I18n::Get(I18n::Option::AssistTickOn),
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemAutoPlaySE), ConfigIni::Key::kAutoPlaySE, Array<StringView>{
				I18n::Get(I18n::Option::Off),
				I18n::Get(I18n::Option::On),
			}),
			CreateInfo::Int(I18n::Get(I18n::Option::ItemMasterVolume), ConfigIni::Key::kMasterVolume, kMasterVolumeMin, kMasterVolumeMax, kMasterVolumeDefault, I18n::Get(I18n::Option::MasterVolumePercent), 5)
				.setOnChangeCallback([]
				{
					const int32 volume = ConfigIni::GetInt(ConfigIni::Key::kMasterVolume, kMasterVolumeDefault);
					ksmaudio::SetMasterVolume(volume / 100.0);
				}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemVsync), ConfigIni::Key::kVsync, Array<StrPair>{
				StrPair{ U"0;120", I18n::Get(I18n::Option::VsyncOffWithFps, 120) },
				StrPair{ U"0;144", I18n::Get(I18n::Option::VsyncOffWithFps, 144) },
				StrPair{ U"0;300", I18n::Get(I18n::Option::VsyncOffWithFps, 300) },
				StrPair{ U"1", I18n::Get(I18n::Option::VsyncOn) },
			}),
		}),
		OptionMenu({
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemDefaultBTJudgmentMode), ConfigIni::Key::kJudgmentModeBT, Array<StringView>{
				I18n::Get(I18n::Option::JudgmentOn),
				I18n::Get(I18n::Option::JudgmentOff),
				I18n::Get(I18n::Option::JudgmentAuto),
				I18n::Get(I18n::Option::JudgmentHide),
			}).setOnChangeCallback([]() {
				RuntimeConfig::SetJudgmentPlayModeBT(static_cast<JudgmentPlayMode>(ConfigIni::GetInt(ConfigIni::Key::kJudgmentModeBT)));
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemDefaultFXJudgmentMode), ConfigIni::Key::kJudgmentModeFX, Array<StringView>{
				I18n::Get(I18n::Option::JudgmentOn),
				I18n::Get(I18n::Option::JudgmentOff),
				I18n::Get(I18n::Option::JudgmentAuto),
				I18n::Get(I18n::Option::JudgmentHide),
			}).setOnChangeCallback([]() {
				RuntimeConfig::SetJudgmentPlayModeFX(static_cast<JudgmentPlayMode>(ConfigIni::GetInt(ConfigIni::Key::kJudgmentModeFX)));
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemDefaultLaserJudgmentMode), ConfigIni::Key::kJudgmentModeLaser, Array<StringView>{
				I18n::Get(I18n::Option::JudgmentOn),
				I18n::Get(I18n::Option::JudgmentOff),
				I18n::Get(I18n::Option::JudgmentAuto),
				I18n::Get(I18n::Option::JudgmentHide),
			}).setOnChangeCallback([]() {
				RuntimeConfig::SetJudgmentPlayModeLaser(static_cast<JudgmentPlayMode>(ConfigIni::GetInt(ConfigIni::Key::kJudgmentModeLaser)));
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemLaserInputType), ConfigIni::Key::kLaserInputType, Array<IntStrPair>{
				IntStrPair{ ConfigIni::Value::LaserInputType::kKeyboard, I18n::Get(I18n::Option::LaserInputTypeKeyboard) },
				IntStrPair{ ConfigIni::Value::LaserInputType::kSlider, I18n::Get(I18n::Option::LaserInputTypeSlider) },
#if !defined(__APPLE__) // macOSではマウスデバイスの取得に入力監視の許可が必要で、配布アプリケーション用に正式なコード署名が必要になる(要Apple Developer Program加入)とみられるため実装済みだが一旦除外
				IntStrPair{ ConfigIni::Value::LaserInputType::kMouseXY, I18n::Get(I18n::Option::LaserInputTypeMouseXY) },
#endif
				IntStrPair{ ConfigIni::Value::LaserInputType::kAnalogStickXY, I18n::Get(I18n::Option::LaserInputTypeAnalogStickXY) },
			}).setOnChangeCallback([]() {
				InputUtils::InitKsmaxisForCurrentLaserInput();
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemDisableIME), ConfigIni::Key::kDisableIME, Array<StringView>{
				I18n::Get(I18n::Option::DisableIMEOff),
				I18n::Get(I18n::Option::DisableIMEOnLow),
				I18n::Get(I18n::Option::DisableIMEOnMid),
				I18n::Get(I18n::Option::DisableIMEOnHigh),
			}),
			CreateInfo::Int(I18n::Get(I18n::Option::ItemTimingAdjustment), ConfigIni::Key::kInputDelay, kTimingAdjustMin, kTimingAdjustMax, kTimingAdjustDefault, I18n::Get(I18n::Option::TimingAdjustMs))
				.setAdditionalSuffixes(
					I18n::Get(I18n::Option::TimingAdjustSuffixNoAdjustment),
					I18n::Get(I18n::Option::TimingAdjustSuffixLater),
					I18n::Get(I18n::Option::TimingAdjustSuffixEarlier)),
			CreateInfo::Int(I18n::Get(I18n::Option::ItemLaserTimingAdjustment), ConfigIni::Key::kLaserInputDelay, kTimingAdjustMin, kTimingAdjustMax, kTimingAdjustDefault, I18n::Get(I18n::Option::TimingAdjustMs))
				.setAdditionalSuffixes(
					I18n::Get(I18n::Option::LaserTimingAdjustSuffixNoAdjustment),
					I18n::Get(I18n::Option::LaserTimingAdjustSuffixLater),
					I18n::Get(I18n::Option::LaserTimingAdjustSuffixEarlier)),
			CreateInfo::Int(I18n::Get(I18n::Option::ItemVisualOffset), ConfigIni::Key::kVisualOffset, kTimingAdjustMin, kTimingAdjustMax, kTimingAdjustDefault, I18n::Get(I18n::Option::TimingAdjustMs))
				.setAdditionalSuffixes(
					I18n::Get(I18n::Option::VisualOffsetSuffixNoAdjustment),
					I18n::Get(I18n::Option::VisualOffsetSuffixLater),
					I18n::Get(I18n::Option::VisualOffsetSuffixEarlier)),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemMouseXInputDirection), ConfigIni::Key::kLaserMouseDirectionX, Array<StringView>{
				I18n::Get(I18n::Option::LaserMouseDirectionLeftThenRight),
				I18n::Get(I18n::Option::LaserMouseDirectionRightThenRight),
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemMouseYInputDirection), ConfigIni::Key::kLaserMouseDirectionY, Array<StringView>{
				I18n::Get(I18n::Option::LaserMouseDirectionUpThenRight),
				I18n::Get(I18n::Option::LaserMouseDirectionDownThenRight),
			}),
			CreateInfo::Int(I18n::Get(I18n::Option::ItemSliderMouseInputSensitivity), ConfigIni::Key::kLaserInputSensitivity, kLaserInputSensitivityMin, kLaserInputSensitivityMax, kLaserInputSensitivityDefault), // TODO: additional suffix for zero value
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemSwapLRLaser), ConfigIni::Key::kSwapLaserLR, Array<StringView>{
				I18n::Get(I18n::Option::Off),
				I18n::Get(I18n::Option::On),
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemCloseDirectoryKey), ConfigIni::Key::kSelectCloseFolderKey, Array<StringView>{
				I18n::Get(I18n::Option::SelectCloseFolderKeyBackspace),
				I18n::Get(I18n::Option::SelectCloseFolderKeyEsc),
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::Item3BTKeysStartBack), ConfigIni::Key::kUse3BTsPlusStartAsBack, Array<StringView>{
				I18n::Get(I18n::Option::Off),
				I18n::Get(I18n::Option::On),
			}),
		}),
		OptionMenu({
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemHispeedTypeXMod), ConfigIni::Key::kHispeedShowXMod, Array<StringView>{
				I18n::Get(I18n::Option::HispeedTypeHide),
				I18n::Get(I18n::Option::HispeedTypeShow),
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemHispeedTypeOMod), ConfigIni::Key::kHispeedShowOMod, Array<StringView>{
				I18n::Get(I18n::Option::HispeedTypeHide),
				I18n::Get(I18n::Option::HispeedTypeShow),
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemHispeedTypeCMod), ConfigIni::Key::kHispeedShowCMod, Array<StringView>{
				I18n::Get(I18n::Option::HispeedTypeHide),
				I18n::Get(I18n::Option::HispeedTypeShow),
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemHideMouseCursor), ConfigIni::Key::kHideMouseCursor, Array<StringView>{
				I18n::Get(I18n::Option::HideMouseCursorOff),
				I18n::Get(I18n::Option::HideMouseCursorOn),
			}),
			CreateInfo::Enum(I18n::Get(I18n::Option::ItemUseNumpadAsArrowKeys), ConfigIni::Key::kUseNumpadAsArrowKeys, Array<StringView>{
				I18n::Get(I18n::Option::UseNumpadAsArrowKeysOff),
				I18n::Get(I18n::Option::UseNumpadAsArrowKeysOnKeyboard),
				I18n::Get(I18n::Option::UseNumpadAsArrowKeysOnController),
			}),
		}),
		OptionMenu({}),
	});
}

OptionScene::OptionScene()
	: m_canvas(LoadOptionSceneCanvas())
	, m_optionMenus(makeOptionMenus())
{
	m_bgmStream.play();
	AutoMuteAddon::SetEnabled(true);
}

void OptionScene::update()
{
	// メニュー更新
	if (m_currentOptionMenuIdx.has_value())
	{
		if (m_currentOptionMenuIdx == OptionMenuType::kKeyConfig)
		{
			m_keyConfigMenu.update();
		}
		else
		{
			if ((*m_optionMenus)[*m_currentOptionMenuIdx].update())
			{
				refreshSettingItemParams();
			}
		}

		if (KeyConfig::Down(kButtonBack))
		{
			// ボタン編集中の場合は戻らない
			if (m_currentOptionMenuIdx == OptionMenuType::kKeyConfig && m_keyConfigMenu.isButtonEditingState())
			{
				// 何もしない
			}
			else
			{
				m_currentOptionMenuIdx = none;
			}
		}
	}
	else
	{
		m_topMenu.update();

		if (KeyConfig::Down(kButtonStart))
		{
			m_currentOptionMenuIdx = m_topMenu.cursorAs<OptionMenuType>();

			// キーコンフィグ以外の設定メニューを開いた場合、設定項目ノードを作成
			if (m_currentOptionMenuIdx != OptionMenuType::kKeyConfig)
			{
				createSettingItemNodes();
			}
		}
		else if (KeyConfig::Down(kButtonBack))
		{
			exitScene();
		}
	}

	// 画面状態の判定
	const bool isTop = !m_currentOptionMenuIdx.has_value();
	const bool isKeyConfig = m_currentOptionMenuIdx == OptionMenuType::kKeyConfig;
	const bool isSetting = m_currentOptionMenuIdx.has_value() && !isKeyConfig;

	// ガイドテキストの決定
	String guideText;
	if (isTop)
	{
		guideText = I18n::Get(I18n::Option::GuideTop);
	}
	else if (isKeyConfig)
	{
		guideText = I18n::Get(I18n::Option::GuideKeyConfig);
	}
	else
	{
		guideText = I18n::Get(I18n::Option::GuideOption);
	}

	const int32 settingHeaderIndex = m_currentOptionMenuIdx.has_value() ? static_cast<int32>(*m_currentOptionMenuIdx) : -1;

	// Canvasパラメータの更新
	m_canvas->setParamValues({
		{ U"isTop", isTop },
		{ U"isSetting", isSetting },
		{ U"isKeyConfig", isKeyConfig },
		{ U"guideText", guideText },
		{ U"settingHeaderIndex", settingHeaderIndex },
		{ U"isJapanese", I18n::CurrentLanguage() == I18n::StandardLanguage::Japanese },
		{ U"isNonJapanese", I18n::CurrentLanguage() != I18n::StandardLanguage::Japanese },
	});

	// UI更新
	m_topMenu.updateUI(m_canvas.get());
	m_keyConfigMenu.updateUI(m_canvas.get());
	m_canvas->update();
}

void OptionScene::draw() const
{
	m_canvas->draw();
}

Co::Task<void> OptionScene::fadeIn()
{
	co_await Co::ScreenFadeIn(kFadeDuration);
}

Co::Task<void> OptionScene::fadeOut()
{
	m_bgmStream.setFadeOut(kFadeDuration);
	co_await Co::ScreenFadeOut(kFadeDuration);
}

void OptionScene::exitScene()
{
	ConfigIni::Save();

	// Vsync設定を反映("0;120"または"1"の形式)
	const Array<String> vsyncParts = String{ ConfigIni::GetString(ConfigIni::Key::kVsync, U"0;300") }.split(U';');
	const bool vsyncEnabled = vsyncParts[0] == U"1";
	Graphics::SetVSyncEnabled(vsyncEnabled);

	// フレームレート制限(Vsync有効時は無効化)
	const int32 fpsLimitValue = vsyncParts.size() >= 2 ? ParseOr<int32>(vsyncParts[1], 300) : 300;
	const Optional<int32> frameRateLimit = vsyncEnabled ? none : Optional<int32>(fpsLimitValue);
	Addon::GetAddon<FrameRateLimit>(FrameRateLimit::kAddonName)->setTargetFPS(frameRateLimit);

	// 画面サイズ反映
	ApplyScreenSizeConfig();

	// シーン遷移
	requestNextScene<TitleScene>(TitleMenuItem::kOption);
}

void OptionScene::refreshOptionMenusForLanguageChange()
{
	// 各メニューのカーソル位置を保存
	std::array<int32, kOptionMenuTypeEnumCount> cursorPositions;
	for (size_t i = 0; i < kOptionMenuTypeEnumCount; ++i)
	{
		cursorPositions[i] = (*m_optionMenus)[i].cursor();
	}

	// メニューを再生成
	m_optionMenus = makeOptionMenus();

	// カーソル位置を復元
	for (size_t i = 0; i < kOptionMenuTypeEnumCount; ++i)
	{
		(*m_optionMenus)[i].setCursor(cursorPositions[i]);
	}

	// UIを再作成
	createSettingItemNodes();
}

void OptionScene::createSettingItemNodes()
{
	const auto containerNode = m_canvas->findByName(U"SettingMenu");
	if (!containerNode)
	{
		Logger << U"[ksm error] OptionScene::createSettingItemNodes(): Container node 'SettingMenu' not found!";
		return;
	}

	// 既存の子を削除
	containerNode->removeChildrenAll();

	// 各設定項目をSubCanvasとして追加
	for (const auto& params : (*m_optionMenus)[*m_currentOptionMenuIdx].getSettingItemParamsList())
	{
		containerNode->addSubCanvasNodeAsChild(
			U"ui/parts/option_setting_item.noco",
			{
				{ U"name", params.name },
				{ U"value", params.value },
				{ U"selected", params.selected },
				{ U"valueArrowIndex", params.valueArrowIndex },
			});
	}
}

void OptionScene::refreshSettingItemParams()
{
	const auto containerNode = m_canvas->findByName(U"SettingMenu");
	if (!containerNode)
	{
		Logger << U"[ksm error] OptionScene::refreshSettingItemParams(): Container node 'SettingMenu' not found!";
		return;
	}

	const auto paramsList = (*m_optionMenus)[*m_currentOptionMenuIdx].getSettingItemParamsList();
	const auto& children = containerNode->children();

	if (paramsList.size() != children.size())
	{
		Logger << U"[ksm error] OptionScene::refreshSettingItemParams(): Children count mismatch! (params={}, children={})"_fmt(paramsList.size(), children.size());
		return;
	}

	for (size_t i = 0; i < paramsList.size(); ++i)
	{
		const auto& params = paramsList[i];
		const auto subCanvas = children[i]->getComponent<noco::SubCanvas>();
		if (!subCanvas)
		{
			Logger << U"[ksm error] OptionScene::refreshSettingItemParams(): SubCanvas component not found at index {}!"_fmt(i);
			continue;
		}

		const auto canvas = subCanvas->canvas();
		if (!canvas)
		{
			Logger << U"[ksm error] OptionScene::refreshSettingItemParams(): Canvas not loaded at index {}!"_fmt(i);
			continue;
		}

		canvas->setParamValues({
			{ U"name", params.name },
			{ U"value", params.value },
			{ U"selected", params.selected },
			{ U"valueArrowIndex", params.valueArrowIndex },
		});
	}
}
