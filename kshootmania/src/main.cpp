#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include "scene/title/title_scene.hpp"
#include "scene/option/option_scene.hpp"
#include "scene/select/select_scene.hpp"
#include "scene/play/play_scene.hpp"
#include "ksmaudio/ksmaudio.hpp"

namespace
{
	// 最大300fpsに制限
	constexpr double kFrameRateLimitFPS = 300.0;

	template <class Clock, class Duration>
	void FrameRateLimit(std::chrono::time_point<Clock, Duration> time)
	{
		const auto nextFrameTime = time + 1000ms / kFrameRateLimitFPS;
		std::this_thread::sleep_until(nextFrameTime - 2ms);
		while (nextFrameTime >= Clock::now());
	}
}

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

	Graphics::SetVSyncEnabled(false);

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

		auto time = std::chrono::steady_clock::now();

		// メインループ
		while (System::Update())
		{
			if (!sceneManager.update())
			{
				break;
			}

			// フレームレート制限
			// (これだと描画反映が1フレーム分遅れてしまうので、本来はSystem::Update内のRenderer::presentより後ろで呼びたい)
			FrameRateLimit(time);
			time = std::chrono::steady_clock::now();
		}
	}

	// config.iniを保存
	ConfigIni::Save();

	// 音声のバックエンドを終了
	ksmaudio::Terminate();
}
