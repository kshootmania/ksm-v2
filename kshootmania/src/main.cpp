#include <Siv3D.hpp>
#include "common/frame_rate_limit.hpp"
#include "ksmaudio/ksmaudio.hpp"

void Main()
{
	// Escキーによるプログラム終了を無効化
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

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

	// 言語ファイルを読み込み
	I18n::LoadLanguage(U"Japanese");

	// config.iniを読み込み
	ConfigIni::Load();

	// アセット一覧を登録
	AssetManagement::RegisterAssets();

	// フレームレート制限
	Graphics::SetVSyncEnabled(false);
	Addon::Register(U"FrameRateLimit", std::make_unique<FrameRateLimit>(300));

	// シーン管理
	Addon::Register(U"SceneManager", std::make_unique<SceneManagerAddon>());

#ifdef _DEBUG
	// ライブラリ側のデバッグ用にコンソール表示(Debugビルドの場合のみ)
	Console.open();
#endif

	// メインループ
	while (System::Update())
	{
	}

	// config.iniを保存
	ConfigIni::Save();

	// 音声のバックエンドを終了
	ksmaudio::Terminate();
}
