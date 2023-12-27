﻿#include <Siv3D.hpp>
#include "common/frame_rate_limit.hpp"
#include "common/ime_utils.hpp"
#include "ksmaudio/ksmaudio.hpp"

void CreateHighScoreBackup()
{
	// 既にバックアップフォルダが存在する場合は何もしない
	if (FileSystem::Exists(U"score_backup"))
	{
		return;
	}

	// scoreフォルダを再帰的にコピー
	FileSystem::Copy(U"score", U"score_backup", CopyOption::UpdateExisting);
}

void Main()
{
	// Escキーによるプログラム終了を無効化
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	// ウィンドウタイトル
	Window::SetTitle(U"K-Shoot MANIA v2.0.0-prealpha");

	// 実行ファイルのパスをカレントディレクトリに設定
	// (ChangeCurrentDirectoryはここ以外は基本的に使用禁止。どうしても使う必要がある場合は必ずModulePathに戻すこと)
	FileSystem::ChangeCurrentDirectory(FileSystem::ModulePath());

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

	// config.iniを読み込み
	ConfigIni::Load();

	// 言語ファイルを読み込み
	I18n::LoadLanguage(ConfigIni::GetString(ConfigIni::Key::kLanguage));

	// ハイスコアのバックアップを作成
	CreateHighScoreBackup();

	// 画面サイズ反映
	Window::SetToggleFullscreenEnabled(false); // Alt+Enter無効化
	ApplyScreenSizeConfig();

	// アセット一覧を登録
	AssetManagement::RegisterAssets();

	// フレームレート制限
	Graphics::SetVSyncEnabled(false);
	Addon::Register(U"FrameRateLimit", std::make_unique<FrameRateLimit>(300), -100);

	// シーン管理
	Addon::Register(U"ScreenFade", std::make_unique<ScreenFadeAddon>(), -1);
	Addon::Register(U"SceneManager", std::make_unique<SceneManagerAddon>(), 0);

	// 毎フレーム連続してアセット生成した時の警告を無効化
	// (楽曲選択でのスクロールにおいては、正常系でもテクスチャ読み込みが毎フレーム発生するため)
	Profiler::EnableAssetCreationWarning(false);

#ifdef _WIN32
	// IME無効化
	IMEUtils::DetachIMEContext();
#endif

#ifdef _DEBUG
	// ライブラリ側のデバッグ用にコンソール表示(Debugビルドの場合のみ)
	Console.open();
#endif

	// シーン初期化
	SceneManagerAddon::Init();

	// メインループ
	while (System::Update())
	{
	}

	// config.iniを保存
	ConfigIni::Save();

	// 音声のバックエンドを終了
	ksmaudio::Terminate();
}
