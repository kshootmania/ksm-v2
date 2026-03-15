#include "PlayScene.hpp"
#include "Scenes/Select/SelectScene.hpp"
#include "Scenes/PlayPrepare/PlayPrepareScene.hpp"
#include "Scenes/Result/ResultScene.hpp"
#include "RuntimeConfig.hpp"
#include "MusicGame/HispeedUtils.hpp"
#include "Common/MessageBoxUtils.hpp"

namespace
{
	constexpr Duration kFadeDuration = 0.6s;

	constexpr Duration kPlayFinishFadeOutDuration = 2.4s;

	Array<MusicGame::HispeedType> LoadAvailableHispeedTypesFromConfigIni()
	{
		Array<MusicGame::HispeedType> availableTypes;
		availableTypes.reserve(static_cast<std::size_t>(MusicGame::HispeedType::EnumCount));

		if (ConfigIni::GetBool(ConfigIni::Key::kHispeedShowXMod, true))
		{
			availableTypes.push_back(MusicGame::HispeedType::XMod);
		}
		if (ConfigIni::GetBool(ConfigIni::Key::kHispeedShowOMod, true))
		{
			availableTypes.push_back(MusicGame::HispeedType::OMod);
		}
		if (ConfigIni::GetBool(ConfigIni::Key::kHispeedShowCMod, false))
		{
			availableTypes.push_back(MusicGame::HispeedType::CMod);
		}

		if (availableTypes.empty())
		{
			availableTypes.push_back(MusicGame::HispeedType::OMod);
		}

		return availableTypes;
	}

	MusicGame::HispeedSetting LoadHispeedSettingFromConfigIni()
	{
		return MusicGame::HispeedUtils::FromConfigStringValue(ConfigIni::GetString(ConfigIni::Key::kHispeed));
	}

	MusicGame::GameCreateInfo MakeGameCreateInfo(
		FilePathView chartFilePath,
		MusicGame::IsAutoPlayYN isAutoPlay,
		const Optional<CoursePlayState>& courseState,
		const Optional<MusicGame::TestPlayOption>& testPlayOption)
	{
		MusicGame::GameCreateInfo info =
		{
			.chartFilePath = FilePath{ chartFilePath },
			.playOption = MusicGame::PlayOption
			{
				.gameMode = courseState.has_value() ? MusicGame::GameMode::kCourseMode : MusicGame::GameMode::kNormal,
				.isAutoPlay = isAutoPlay,
				.gaugeType = RuntimeConfig::GetGaugeType(),
				.turnMode = RuntimeConfig::GetTurnMode(),
				.playbackSpeed = RuntimeConfig::GetPlaybackSpeed(),
				.btJudgmentPlayMode = RuntimeConfig::GetJudgmentPlayModeBT(),
				.fxJudgmentPlayMode = RuntimeConfig::GetJudgmentPlayModeFX(),
				.laserJudgmentPlayMode = RuntimeConfig::GetJudgmentPlayModeLaser(),
				.globalOffsetMs = ConfigIni::GetInt(ConfigIni::Key::kGlobalOffset),
				.inputDelayMs = ConfigIni::GetInt(ConfigIni::Key::kInputDelay),
				.laserInputDelayMs = ConfigIni::GetInt(ConfigIni::Key::kLaserInputDelay),
				.audioProcDelayMs = ConfigIni::GetInt(ConfigIni::Key::kAudioProcDelay),
				.visualOffsetMs = ConfigIni::GetInt(ConfigIni::Key::kVisualOffset),
				.isAutoPlaySE = ConfigIni::GetBool(ConfigIni::Key::kAutoPlaySE),
				.noteSkin = [&]()
				{
					const StringView noteSkinStr = ConfigIni::GetString(ConfigIni::Key::kNoteSkin, U"default");
					return noteSkinStr == U"note" ? NoteSkinType::kNote : NoteSkinType::kDefault;
				}(),
				.autoSyncMode = static_cast<AutoSyncMode>(ConfigIni::GetInt(ConfigIni::Key::kAutoSync, static_cast<int32>(AutoSyncMode::kOff))),
				.fastSlowMode = static_cast<FastSlowMode>(ConfigIni::GetInt(ConfigIni::Key::kShowFastSlow, static_cast<int32>(FastSlowMode::kHide))),
				.availableHispeedTypes = LoadAvailableHispeedTypesFromConfigIni(),
				.hispeedSetting = LoadHispeedSettingFromConfigIni(),
				.movieEnabled = ConfigIni::GetInt(ConfigIni::Key::kBGMovie, static_cast<int32>(MovieMode::kOn)) == static_cast<int32>(MovieMode::kOn),
				.showBG = [&]()
				{
					const int32 bgDisplayMode = ConfigIni::GetInt(ConfigIni::Key::kBGDisplayMode, ConfigIni::Value::BGDisplayMode::kShowLayer);
					return bgDisplayMode != ConfigIni::Value::BGDisplayMode::kHide;
				}(),
				.showLayer = [&]()
				{
					const int32 bgDisplayMode = ConfigIni::GetInt(ConfigIni::Key::kBGDisplayMode, ConfigIni::Value::BGDisplayMode::kShowLayer);
					return bgDisplayMode == ConfigIni::Value::BGDisplayMode::kShowLayer;
				}(),
			},
			.assistTickMode = static_cast<AssistTickMode>(ConfigIni::GetInt(ConfigIni::Key::kAssistTick, static_cast<int32>(AssistTickMode::kOff))),
			.courseContinuation = courseState.has_value() && courseState->currentChartIdx() > 0 ? MakeOptional(courseState->continuation()) : none,
			.folderConfIni = FolderConfIni::Load(chartFilePath),
		};

		// テストプレイオプションを適用
		if (testPlayOption.has_value())
		{
			info.playOption.testPlayOption = testPlayOption;
			if (testPlayOption->gaugeType.has_value())
			{
				info.playOption.gaugeType = *testPlayOption->gaugeType;
			}
		}

		return info;
	}
}

PlayScene::PlayScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay, const Optional<CoursePlayState>& courseState, const Optional<MusicGame::TestPlayOption>& testPlayOption)
	: m_gameMain(MakeGameCreateInfo(chartFilePath, isAutoPlay, courseState, testPlayOption))
	, m_isAutoPlay(isAutoPlay)
	, m_courseState(courseState)
	, m_fadeOutDuration(kFadeDuration)
	, m_testPlayOption(testPlayOption)
{
	m_gameMain.start();

	// Playシーンではウィンドウのフォーカスが外れていてもミュートしない
	AutoMuteAddon::SetEnabled(false);
}

PlayScene::~PlayScene()
{
	m_gameMain.terminate();
}

void PlayScene::update()
{
	const auto startFadeOut = m_gameMain.update();

	// Backボタンでリザルト画面に遷移(lockForExit中でも有効)
	processBackButtonInput();

	if (startFadeOut)
	{
		// 譜面終了時にリザルト画面に遷移
		m_fadeOutDuration = kPlayFinishFadeOutDuration;

		if (m_testPlayOption.has_value() && (m_testPlayOption->hasStartMeasure() || m_isAutoPlay))
		{
			// テストプレイ(-from指定ありまたはオートプレイ)の場合はアプリケーション終了
			requestSceneFinish();
		}
		else if (m_testPlayOption.has_value())
		{
			// テストプレイ(-fromなし、手動)の場合はリザルト画面へ
			const ResultSceneArgs args =
			{
				.chartFilePath = FilePath(m_gameMain.chartFilePath()),
				.chartData = m_gameMain.chartData(),
				.playResult = m_gameMain.playResult(),
			};
			requestNextScene<ResultScene>(args);
		}
		else if (m_isAutoPlay)
		{
			// オートプレイの場合
			if (m_courseState && m_courseState->hasNextChart())
			{
				// コースモードで次の曲がある場合
				// リザルト画面をスキップするため、プレイリザルト(次曲へのコンボ引き継ぎに必要)はここで記録
				const MusicGame::PlayResult playResult = m_gameMain.playResult();
				m_courseState->recordResult(playResult);

				// 次の曲へ
				m_courseState->advanceToNextChart();
				const FilePath nextChartPath = m_courseState->currentChartPath();
				requestNextScene<PlayPrepareScene>(nextChartPath, MusicGame::IsAutoPlayYN::Yes, m_courseState);
			}
			else
			{
				// 次の曲がない場合は選曲画面へ
				requestNextScene<SelectScene>();
			}
		}
		else
		{
			showAutoSyncSaveDialog();

			const ResultSceneArgs args =
			{
				.chartFilePath = FilePath(m_gameMain.chartFilePath()),
				.chartData = m_gameMain.chartData(), // TODO: shared_ptrでコピーを避ける?
				.playResult = m_gameMain.playResult(),
				.courseState = m_courseState,
			};
			requestNextScene<ResultScene>(args);
		}
	}
}

void PlayScene::processBackButtonInput()
{
	if (!KeyConfig::Down(kButtonBack))
	{
		return;
	}

	m_fadeOutDuration = 0s;
	m_backButtonPressedDuringFadeOut = true;

	// Backボタンを押した後は以降の処理でスコア変動しないようロック
	m_gameMain.lockForExit();

	// 次のシーンで多重に反応しないよう、Backボタンの入力をクリア
	KeyConfig::ClearInput(kButtonBack);

	if (m_testPlayOption.has_value())
	{
		// テストプレイの場合はアプリケーション終了
		requestSceneFinish();
	}
	else if (m_isAutoPlay)
	{
		requestNextScene<SelectScene>();
	}
	else
	{
		showAutoSyncSaveDialog();

		const ResultSceneArgs args =
		{
			.chartFilePath = FilePath(m_gameMain.chartFilePath()),
			.chartData = m_gameMain.chartData(), // TODO: shared_ptrでコピーを避ける?
			.playResult = m_gameMain.playResult(),
			.courseState = m_courseState,
		};
		requestNextScene<ResultScene>(args);
	}
}

void PlayScene::updateFadeOut()
{
	// フェードアウト中もゲームの更新は継続
	m_gameMain.update();

	// Backボタンでフェードアウトをスキップしてリザルト画面に遷移
	processBackButtonInput();
}

void PlayScene::draw() const
{
	m_gameMain.draw();
}

inline Co::Task<void> PlayScene::fadeIn()
{
	co_await Co::ScreenFadeIn(kFadeDuration);
}

namespace
{
	// KSHファイルのo=フィールドを書き換え
	void UpdateKshOffset(const FilePath& chartFilePath, int32 newOffset)
	{
		TextReader reader(chartFilePath);
		if (!reader)
		{
			Logger << U"[ksm error] AutoSync: Failed to open KSH file for reading: '{}'"_fmt(chartFilePath);
			return;
		}

		Array<String> lines;
		String line;
		bool replaced = false;
		while (reader.readLine(line))
		{
			if (!replaced && line.starts_with(U"o="))
			{
				lines.push_back(U"o={}"_fmt(newOffset));
				replaced = true;
			}
			else
			{
				lines.push_back(line);
			}
		}
		reader.close();

		BinaryWriter writer(chartFilePath);
		if (writer)
		{
			for (std::size_t i = 0; i < lines.size(); ++i)
			{
				const std::string utf8 = lines[i].toUTF8();
				writer.write(utf8.data(), utf8.size());
				writer.write("\r\n", 2);
			}
		}
	}

	// KSONファイルのaudio.bgm.offsetを書き換え
	void UpdateKsonOffset(const FilePath& chartFilePath, int32 newOffset)
	{
		JSON json = JSON::Load(chartFilePath);
		if (!json)
		{
			Logger << U"[ksm error] AutoSync: Failed to load KSON file: '{}'"_fmt(chartFilePath);
			return;
		}

		json[U"audio"][U"bgm"][U"offset"] = newOffset;
		json.save(chartFilePath);
	}
}

void PlayScene::showAutoSyncSaveDialog()
{
	const int32 offsetMs = m_gameMain.timingAdjustOffsetMs();
	if (offsetMs == 0 || m_isAutoPlay || m_testPlayOption.has_value())
	{
		return;
	}

	const int32 currentOffset = m_gameMain.chartData().audio.bgm.offset;
	const int32 newOffset = currentOffset + offsetMs;

	const String offsetStr = U"{:+}"_fmt(offsetMs);
	const String message = I18n::Get(I18n::Play::AutoSyncSaveConfirm, offsetStr, currentOffset, newOffset);

	const auto result = MessageBoxUtils::ShowYesNo(message, MessageBoxStyle::Question);
	if (result == MessageBoxResult::OK)
	{
		const FilePath chartFilePath{ m_gameMain.chartFilePath() };
		if (FsUtils::HasKsonExtension(chartFilePath))
		{
			UpdateKsonOffset(chartFilePath, newOffset);
		}
		else
		{
			UpdateKshOffset(chartFilePath, newOffset);
		}
	}
}

Co::Task<void> PlayScene::fadeOut()
{
	// フェードアウト中もBack入力を受け付けるため、updateFadeOutを実行
	const auto updateRunner = Co::UpdaterTask([this] { updateFadeOut(); }).runScoped();

	m_gameMain.startBGMFadeOut(m_fadeOutDuration);

	co_await Co::Any(
		Co::ScreenFadeOut(m_fadeOutDuration),
		Co::WaitUntil([this] { return m_backButtonPressedDuringFadeOut; }));
}
