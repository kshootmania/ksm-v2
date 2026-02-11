#include <Siv3D.hpp>
#include <CoTaskLib.hpp>
#include <NocoUI.hpp>
#include "Common/FrameRateLimit.hpp"
#include "Common/IMEUtils.hpp"
#include "Common/AssetManagement.hpp"
#include "Addon/AutoMuteAddon.hpp"
#include "Addon/CommonSEAddon.hpp"
#include "Addon/DisableIMEAddon.hpp"
#include "ksmaudio/ksmaudio.hpp"
#include <ksmaxis/ksmaxis.hpp>
#include "RuntimeConfig.hpp"
#include "Scenes/Title/TitleScene.hpp"
#include "Scenes/Play/PlayScene.hpp"
#include "Scenes/PlayPrepare/PlayPrepareScene.hpp"
#include "TestPlayArgs.hpp"
#include "Input/KeyConfig.hpp"
#include "Input/InputUtils.hpp"

#ifdef __APPLE__
#include <ksmplatform_macos/input_method.h>
#endif

void CreateHighScoreBackup()
{
#ifndef __APPLE__
	const FilePath scoreBackupDir = FileSystem::PathAppend(FsUtils::AppDataDirectoryPath(), U"score_backup");

	// 既にバックアップフォルダが存在する場合は何もしない
	if (FileSystem::Exists(scoreBackupDir))
	{
		return;
	}

	// scoreフォルダを再帰的にコピー
	const FilePath scoreDir = FsUtils::ScoreDirectoryPath();
	FileSystem::Copy(scoreDir, scoreBackupDir, CopyOption::UpdateExisting);
#endif
}

void CreateAppDataDirectory()
{
#ifdef __APPLE__
	const FilePath appDataDir = FsUtils::AppDataDirectoryPath();
	Logger << U"[ksm info] AppDataDirectory path: " << appDataDir;

	// ディレクトリが存在しない場合は作成
	if (!FileSystem::Exists(appDataDir))
	{
		Logger << U"[ksm info] Creating AppDataDirectory...";
		FileSystem::CreateDirectories(appDataDir);
		const bool created = FileSystem::Exists(appDataDir);
		Logger << U"[ksm info] Created: " << created;
	}
	else
	{
		Logger << U"[ksm info] AppDataDirectory already exists";
	}
#endif
}

void CopyResourcesIfNeeded()
{
#ifdef __APPLE__
	const FilePath songsDestPath = FsUtils::SongsDirectoryPath();
	const FilePath songsDefaultSrcPath = FsUtils::SongsDefaultDirectoryPath();

	Logger << U"[ksm info] Songs dest path: " << songsDestPath;
	Logger << U"[ksm info] Songs default src path: " << songsDefaultSrcPath;
	Logger << U"[ksm info] Songs dest exists: " << FileSystem::Exists(songsDestPath);
	Logger << U"[ksm info] Songs default src exists: " << FileSystem::Exists(songsDefaultSrcPath);

	// songsフォルダが存在しない場合、songs_defaultからコピー
	if (!FileSystem::Exists(songsDestPath))
	{
		if (FileSystem::Exists(songsDefaultSrcPath))
		{
			Logger << U"[ksm info] Copying songs_default to songs...";

			// Siv3DのFileSystem::CopyだとApplication Support内へのファイルコピーに失敗するためネイティブライブラリを使用
			const std::string srcPathUtf8 = songsDefaultSrcPath.toUTF8();
			const std::string dstPathUtf8 = songsDestPath.toUTF8();

			const bool copyResult = KSMPlatformMacOS_CopyDirectory(srcPathUtf8.c_str(), dstPathUtf8.c_str());
			Logger << U"[ksm info] Copy result: " << copyResult;
			Logger << U"[ksm info] Songs dest exists after copy: " << FileSystem::Exists(songsDestPath);

			// コピー後のディレクトリ内容を確認
			if (FileSystem::Exists(songsDestPath))
			{
				const auto copiedFiles = FileSystem::DirectoryContents(songsDestPath);
				Logger << U"[ksm info] Files in songs directory: " << copiedFiles.size();
			}
		}
		else
		{
			Logger << U"[ksm info] songs_default does not exist, skipping copy";
		}
	}
	else
	{
		Logger << U"[ksm info] songs directory already exists, skipping copy";
	}
#endif
}

void OutputLicenseTxt()
{
	// ライセンス情報を取得
	Array<LicenseInfo> licenses = LicenseManager::EnumLicenses();

	// BASS Audio Libraryのライセンス情報を追加
	LicenseInfo bassLicense;
	bassLicense.title = U"BASS Audio Library";
	bassLicense.copyright = U"Copyright (c) 1999-2024 Un4seen Developments Ltd.\nhttps://www.un4seen.com/";
	bassLicense.text = U"BASS is free for non-commercial use.\nIf you are using BASS in a commercial product, you must obtain a license.";
	licenses.push_back(bassLicense);

	// BASS_FXのライセンス情報を追加
	LicenseInfo bassFxLicense;
	bassFxLicense.title = U"BASS_FX";
	bassFxLicense.copyright = U"Copyright (c) 2002-2018 Arthur Aminov (JOBnik!)\nhttp://www.jobnik.org";
	bassFxLicense.text = U"BASS_FX is fully useable in commercial software, as long as credit is given.\n"
		U"\n"
		U"Credits:\n"
		U"* BiQuad filters - Robert Bristow-Johnson\n"
		U"* Peaking Equalizer (BiQuad filter) - Manu Webber\n"
		U"* Tempo/Pitch/Rate/BPM [SoundTouch v2.0.0] - Copyright (c) 2002-2017 Olli Parviainen (LGPL license)\n"
		U"* Auto Wah, Chorus, Distortion, Echo and Phaser - Copyright (c) 2000 Aleksey Smoli\n"
		U"* Freeverb - Copyright (c) 2000 Jezar at Dreampoint (Public domain)\n"
		U"* Pitch shifting using FFT [smbPitchShift v1.2] - Copyright (c) 1999-2009 Stephan M. Bernsee";
	licenses.push_back(bassFxLicense);

	// libksonのライセンス情報を追加
	LicenseInfo ksonLicense;
	ksonLicense.title = U"libkson";
	ksonLicense.copyright = U"Copyright (c) 2019-2022 masaka";
	ksonLicense.text = U"Permission is hereby granted, free of charge, to any person obtaining a copy\n"
		U"of this software and associated documentation files (the \"Software\"), to deal\n"
		U"in the Software without restriction, including without limitation the rights\n"
		U"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
		U"copies of the Software, and to permit persons to whom the Software is\n"
		U"furnished to do so, subject to the following conditions:\n"
		U"\n"
		U"The above copyright notice and this permission notice shall be included in all\n"
		U"copies or substantial portions of the Software.\n"
		U"\n"
		U"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
		U"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
		U"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
		U"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
		U"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
		U"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
		U"SOFTWARE.";
	licenses.push_back(ksonLicense);

	// ksmaxisのライセンス情報を追加
	LicenseInfo ksmaxisLicense;
	ksmaxisLicense.title = U"ksmaxis";
	ksmaxisLicense.copyright = U"Copyright (c) 2025 masaka";
	ksmaxisLicense.text = U"Permission is hereby granted, free of charge, to any person obtaining a copy\n"
		U"of this software and associated documentation files (the \"Software\"), to deal\n"
		U"in the Software without restriction, including without limitation the rights\n"
		U"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
		U"copies of the Software, and to permit persons to whom the Software is\n"
		U"furnished to do so, subject to the following conditions:\n"
		U"\n"
		U"The above copyright notice and this permission notice shall be included in all\n"
		U"copies or substantial portions of the Software.\n"
		U"\n"
		U"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
		U"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
		U"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
		U"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
		U"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
		U"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
		U"SOFTWARE.";
	licenses.push_back(ksmaxisLicense);

	// KSMフォントのライセンス情報を追加
	LicenseInfo ksmFontLicense;
	ksmFontLicense.title = U"KSM Fonts (KSM-JA/SC/TC-Medium)";
	ksmFontLicense.copyright = U"Component fonts:\n"
		U"1. Tektur (Modified as Tektur-KSM) - Copyright 2023 The Tektur Project Authors, Modified by K-Shoot MANIA Project\n"
		U"2. Corporate Logo ver3 - Copyright LOGOTYPE.JP, Based on Source Han Sans (Copyright 2014-2020 Adobe)\n"
		U"3. Noto Sans, Noto Sans JP/KR/SC/TC, Noto Sans Math, Noto Sans Symbols, Noto Music, Arabic, Thai, Hebrew, Cherokee - Copyright Google Inc. and Adobe Inc.";
	ksmFontLicense.text = U"All component fonts are licensed under the SIL Open Font License, Version 1.1.\n"
		U"\n"
		U"Permission is hereby granted, free of charge, to any person obtaining a copy of the Font Software,\n"
		U"to use, study, copy, merge, embed, modify, redistribute, and sell modified and unmodified copies\n"
		U"of the Font Software, subject to the following conditions:\n"
		U"\n"
		U"1) Neither the Font Software nor any of its individual components, in Original or Modified Versions,\n"
		U"   may be sold by itself.\n"
		U"2) Original or Modified Versions of the Font Software may be bundled, redistributed and/or sold with\n"
		U"   any software, provided that each copy contains the above copyright notice and this license.\n"
		U"3) The Font Software, modified or unmodified, in part or in whole, must be distributed entirely under\n"
		U"   this license, and must not be distributed under any other license.";
	licenses.push_back(ksmFontLicense);

	// アルファベット順にソート
	licenses.sort_by([](const LicenseInfo& a, const LicenseInfo& b)
	{
		return a.title < b.title;
	});

	// ファイルに保存
	TextWriter writer(U"license.txt");
	if (!writer)
	{
		Logger << U"[ksm error] Failed to output license.txt";
		return;
	}

	writer.writeln(U"●使用ライブラリのライセンス表記●");

	for (const auto& license : licenses)
	{
		writer.writeln(U"====================");
		writer.writeln(license.title);
		writer.writeln(U"====================");
		if (license.copyright)
		{
			writer.writeln(license.copyright);
			writer.writeln();
		}
		writer.writeln(license.text);
		writer.writeln();
	}
}

void KSMMain()
{
#if defined(__linux__)
	FsUtils::InitModulePathForLinux();
#endif

	// Escキーによるプログラム終了を無効化
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	// F1キーによるライセンス表示を無効化
	LicenseManager::DisableDefaultTrigger();

	// ウィンドウタイトル
	Window::SetTitle(U"K-Shoot MANIA v2.0.0-alpha5");

	// カレントディレクトリを設定
	// (ChangeCurrentDirectoryはここ以外は基本的に使用禁止。どうしても使う必要がある場合は必ずResourceDirectoryPathに戻すこと)
	FileSystem::ChangeCurrentDirectory(FsUtils::ResourceDirectoryPath());

	// デフォルト色を指定
	Scene::SetBackground(Palette::Black);
	Graphics3D::SetGlobalAmbientColor(Palette::White);
	Graphics3D::SetSunColor(Palette::Black);

	// 音声処理のバックエンドを初期化
#ifdef _WIN32
	ksmaudio::Init(s3d::Platform::Windows::Window::GetHWND());
#else
	ksmaudio::Init(nullptr);
#endif

	// アプリケーションデータディレクトリを作成(macOSのみ)
	CreateAppDataDirectory();

	// リソースファイルをコピー(macOSのみ)
	CopyResourcesIfNeeded();

	// config.iniを読み込み
	ConfigIni::Load();

	// ランタイム設定を初期化
	RuntimeConfig::RestoreJudgmentModesFromConfigIni();

	// マスターボリュームを適用
	{
		constexpr int32 kMasterVolumeDefault = 100;
		const int32 masterVolume = ConfigIni::GetInt(ConfigIni::Key::kMasterVolume, kMasterVolumeDefault);
		ksmaudio::SetMasterVolume(masterVolume / 100.0);
	}

	// 言語ファイルを読み込み
	I18n::LoadLanguage(ConfigIni::GetString(ConfigIni::Key::kLanguage));

	// ハイスコアのバックアップを作成
	CreateHighScoreBackup();

	// 画面サイズ反映
	Window::SetToggleFullscreenEnabled(false); // Alt+Enter無効化
	ApplyScreenSizeConfig();

	// アセット一覧を登録
	AssetManagement::RegisterAssets();

	// Vsync設定を反映("0;120"または"1"の形式)
	const Array<String> vsyncParts = String{ ConfigIni::GetString(ConfigIni::Key::kVsync, U"0;300") }.split(U';');
	const bool vsyncEnabled = vsyncParts[0] == U"1";
	Graphics::SetVSyncEnabled(vsyncEnabled);

	// フレームレート制限(Vsync有効時は無効化)
	const int32 fpsLimitValue = vsyncParts.size() >= 2 ? ParseOr<int32>(vsyncParts[1], 300) : 300;
	const Optional<int32> frameRateLimit = vsyncEnabled ? none : Optional<int32>(fpsLimitValue);
	Addon::Register(FrameRateLimit::kAddonName, std::make_unique<FrameRateLimit>(frameRateLimit), -100);

	Addon::Register(AutoMuteAddon::kAddonName, std::make_unique<AutoMuteAddon>(), 1);

	Addon::Register(CommonSEAddon::kAddonName, std::make_unique<CommonSEAddon>(), 2);

#if defined(_WIN32) || defined(__APPLE__)
	Addon::Register(DisableIMEAddon::kAddonName, std::make_unique<DisableIMEAddon>(), 3);
#endif

	// 毎フレーム連続してアセット生成した時の警告を無効化
	// (楽曲選択でのスクロールにおいては、正常系でもテクスチャ読み込みが毎フレーム発生するため)
	Profiler::EnableAssetCreationWarning(false);

#if defined(_WIN32) || defined(__APPLE__)
	// 起動時はIME無効化を一度実行
	// (これは起動時のものなので、IME無効化設定とは関係なく必ず実行)
	IMEUtils::DetachIMEContext();
#endif

#ifdef __APPLE__
	// macOS: 英数・かなキーのイベントタップを開始してIME切り替えを防ぐ
	KSMPlatformMacOS_StartBlockingIMEKeys();
#endif

#ifdef _DEBUG
	// ライブラリ側のデバッグ用にコンソール表示(Debugビルドの場合のみ)
	Console.open();
#endif

	// ライブラリ初期化
	Co::Init();
	noco::Init();

	// レーザー入力方式がキーボード以外なら、ksmaxisを初期化
	InputUtils::InitKsmaxisForCurrentLaserInput();

	// NocoUIのグローバルデフォルトフォントを設定
	noco::SetGlobalDefaultFont(AssetManagement::SystemFont());

#ifdef OUTPUT_LICENSE_TXT
	OutputLicenseTxt();
#endif

	// コマンドライン引数をパース
	bool shouldExit = false;
	const auto testPlayArgs = ParseTestPlayArgs(&shouldExit);
	if (shouldExit)
	{
		return;
	}

	// テストプレイの場合、譜面ファイルの読み込みを事前検証
	if (testPlayArgs.has_value())
	{
		const auto chartData = kson::LoadKshChartData(testPlayArgs->chartFilePath.narrow());
		if (chartData.error != kson::ErrorType::None)
		{
			System::MessageBoxOK(I18n::Get(I18n::Play::ErrorChartLoadFailed), MessageBoxStyle::Error);
			return;
		}
	}

	// 開始シーンを決定
	Co::SceneFactory initialSceneFactory;
	if (testPlayArgs.has_value() && testPlayArgs->testPlayOption.hasStartMeasure())
	{
		// テストプレイ(-from指定あり)の場合
		initialSceneFactory = Co::MakeSceneFactory<PlayScene>(
			testPlayArgs->chartFilePath,
			testPlayArgs->isAutoPlay,
			Optional<CoursePlayState>{ none },
			MakeOptional(testPlayArgs->testPlayOption));
	}
	else if (testPlayArgs.has_value())
	{
		// テストプレイ(-fromなし)の場合
		initialSceneFactory = Co::MakeSceneFactory<PlayPrepareScene>(
			testPlayArgs->chartFilePath,
			testPlayArgs->isAutoPlay,
			Optional<CoursePlayState>{ none },
			MakeOptional(testPlayArgs->testPlayOption));
	}
	else
	{
		// 通常起動
		initialSceneFactory = Co::MakeSceneFactory<TitleScene>(TitleMenuItem::kStart);
	}

	// メインループ
	const auto sceneRunner = Co::PlaySceneFrom(std::move(initialSceneFactory)).runScoped();
	
	while (System::Update())
	{
		// マウスカーソル非表示設定
		if (ConfigIni::GetBool(ConfigIni::Key::kHideMouseCursor))
		{
			Cursor::RequestStyle(CursorStyle::Hidden);
		}

		if (ksmaxis::IsInitialized())
		{
			ksmaxis::Update();
		}

#ifdef __APPLE__
		// macOSプラットフォーム特有のキーボード状態を更新
		KeyConfig::UpdatePlatformKeyboard();

		// テキスト編集中かどうかを設定(テキスト編集中はIMEキーをブロックしない)
		KSMPlatformMacOS_SetIsEditingText(noco::IsEditingTextBox());
#endif

		if (sceneRunner.done())
		{
			break;
		}
	}

	// config.iniを保存
	ConfigIni::Save();

#ifdef __APPLE__
	// macOS: 英数・かなキーのイベントタップを停止
	KSMPlatformMacOS_StopBlockingIMEKeys();
#endif

	// ライブラリ終了
	if (ksmaxis::IsInitialized())
	{
		ksmaxis::Terminate();
	}
	ksmaudio::Terminate();
}

void Main()
{
	try
	{
		KSMMain();
	}
	catch (const Error& e)
	{
		System::MessageBoxOK(e.what(), MessageBoxStyle::Error);
		throw;
	}
}
