#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include "scene/title/title_scene.hpp"
#include "scene/option/option_scene.hpp"
#include "scene/select/select_scene.hpp"
#include "scene/play/play_scene.hpp"
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

	//Graphics::SetVSyncEnabled(false);

#if defined(_WIN32) && defined(_DEBUG)
	AllocConsole();
	FILE* fp = NULL;
	freopen_s(&fp, "CONOUT$", "w", stdout);
#endif

	{
		// 各シーンを作成
		MySceneManager sceneManager;
		sceneManager.add<TitleScene>(SceneName::kTitle);
		sceneManager.add<OptionScene>(SceneName::kOption);
		sceneManager.add<SelectScene>(SceneName::kSelect);
		sceneManager.add<PlayScene>(SceneName::kPlay);
		sceneManager.changeScene(SceneName::kTitle, kDefaultTransitionMs);

		// メインループ
		while (System::Update())
		{
			KeyConfig::Update();
			if (!sceneManager.update())
			{
				break;
			}
		}
	}

	// config.iniを保存
	ConfigIni::Save();

	// 音声のバックエンドを終了
	ksmaudio::Terminate();
}
