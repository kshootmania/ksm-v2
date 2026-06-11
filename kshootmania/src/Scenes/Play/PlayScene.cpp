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
				.isAutoPlaySE = isAutoPlay || ConfigIni::GetBool(ConfigIni::Key::kAutoPlaySE),
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

PlayScene::PlayScene(FilePathView chartFilePath, MusicGame::IsAutoPlayYN isAutoPlay, const Optional<CoursePlayState>& courseState, const Optional<MusicGame::TestPlayOption>& testPlayOption, const Optional<SelectSceneSearchParams>& selectSearchParams)
	: m_gameMain(MakeGameCreateInfo(chartFilePath, isAutoPlay, courseState, testPlayOption))
	, m_isAutoPlay(isAutoPlay)
	, m_courseState(courseState)
	, m_fadeOutDuration(kFadeDuration)
	, m_testPlayOption(testPlayOption)
	, m_selectSearchParams(selectSearchParams)
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
				.selectSearchParams = m_selectSearchParams,
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
				requestNextScene<PlayPrepareScene>(nextChartPath, MusicGame::IsAutoPlayYN::Yes, m_courseState, none, m_selectSearchParams);
			}
			else
			{
				// 次の曲がない場合は選曲画面へ
				requestNextScene<SelectScene>(m_selectSearchParams);
			}
		}
		else
		{
			const ResultSceneArgs args =
			{
				.chartFilePath = FilePath(m_gameMain.chartFilePath()),
				.chartData = m_gameMain.chartData(), // TODO: shared_ptrでコピーを避ける?
				.playResult = m_gameMain.playResult(),
				.courseState = m_courseState,
				.selectSearchParams = m_selectSearchParams,
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
			.selectSearchParams = m_selectSearchParams,
		};
		requestNextScene<ResultScene>(args);
	}
	else if (m_isAutoPlay)
	{
		requestNextScene<SelectScene>(m_selectSearchParams);
	}
	else
	{
		const ResultSceneArgs args =
		{
			.chartFilePath = FilePath(m_gameMain.chartFilePath()),
			.chartData = m_gameMain.chartData(), // TODO: shared_ptrでコピーを避ける?
			.playResult = m_gameMain.playResult(),
			.courseState = m_courseState,
			.selectSearchParams = m_selectSearchParams,
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
	// KSHファイルの"o="の行を書き換え
	void ReplaceKshOffset(const FilePath& chartFilePath, int32 newOffset)
	{
		BinaryReader reader{ chartFilePath };
		if (!reader)
		{
			Logger << U"[ksm error] AutoSync: Failed to open KSH file for reading '{}'"_fmt(chartFilePath);
			return;
		}

		std::string content(static_cast<std::size_t>(reader.size()), '\0');
		if (!content.empty())
		{
			reader.read(content.data(), static_cast<int64>(content.size()));
		}
		reader.close();

		std::size_t searchStart = 0;
		if (content.size() >= 3 &&
			static_cast<uint8_t>(content[0]) == 0xEF &&
			static_cast<uint8_t>(content[1]) == 0xBB &&
			static_cast<uint8_t>(content[2]) == 0xBF)
		{
			searchStart = 3;
		}

		const auto isOEqualsAt = [&content](std::size_t pos) -> bool
		{
			return pos + 1 < content.size() && content[pos] == 'o' && content[pos + 1] == '=';
		};

		std::size_t lineStart = std::string::npos;
		if (isOEqualsAt(searchStart))
		{
			lineStart = searchStart;
		}
		else
		{
			std::size_t pos = searchStart;
			while (pos < content.size())
			{
				const std::size_t newlinePos = content.find('\n', pos);
				if (newlinePos == std::string::npos)
				{
					break;
				}
				const std::size_t candidate = newlinePos + 1;
				if (isOEqualsAt(candidate))
				{
					lineStart = candidate;
					break;
				}
				pos = newlinePos + 1;
			}
		}

		if (lineStart == std::string::npos)
		{
			Logger << U"[ksm error] AutoSync: 'o=' line not found in KSH file '{}'"_fmt(chartFilePath);
			return;
		}

		std::size_t lineEnd = lineStart;
		while (lineEnd < content.size() && content[lineEnd] != '\r' && content[lineEnd] != '\n')
		{
			++lineEnd;
		}

		// "o="の行を置換
		const std::string newLine = "o=" + std::to_string(newOffset);
		content.replace(lineStart, lineEnd - lineStart, newLine);

		BinaryWriter writer{ chartFilePath };
		if (writer)
		{
			writer.write(content.data(), static_cast<int64>(content.size()));
		}
		else
		{
			Logger << U"[ksm error] AutoSync: Failed to open KSH file for writing '{}'"_fmt(chartFilePath);
		}
	}

	// KSONファイルのaudio.bgm.offsetを書き換え
	void ReplaceKsonOffset(const FilePath& chartFilePath, int32 newOffset)
	{
		JSON json = JSON::Load(chartFilePath);
		if (!json)
		{
			Logger << U"[ksm error] AutoSync: Failed to load KSON file '{}'"_fmt(chartFilePath);
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
			ReplaceKsonOffset(chartFilePath, newOffset);
		}
		else
		{
			ReplaceKshOffset(chartFilePath, newOffset);
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

	showAutoSyncSaveDialog();
}
