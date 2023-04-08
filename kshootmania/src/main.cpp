#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include "scene/title/title_scene.hpp"
#include "scene/option/option_scene.hpp"
#include "scene/select/select_scene.hpp"
#include "scene/play/play_scene.hpp"
#include "ksmaudio/ksmaudio.hpp"

class FrameRateLimit : public IAddon
{
private:
	int32 m_targetFPS;

	std::chrono::time_point<std::chrono::steady_clock> m_sleepUntil;

public:
	explicit FrameRateLimit(int32 targetFPS)
		: m_targetFPS(targetFPS)
		, m_sleepUntil(std::chrono::steady_clock::now())
	{
	}

	virtual void postPresent() override
	{
		// 次フレームまでのsleepの終了時間を決める
		m_sleepUntil += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)) / m_targetFPS;

		// 目標フレームレートに届かなかったときにm_sleepUntilが現在時間より過去に離れていかないよう現在時間以降にする
		const auto sleepUntilMin = std::chrono::steady_clock::now();
		if (m_sleepUntil < sleepUntilMin)
		{
			m_sleepUntil = sleepUntilMin;
		}

		// sleepを実行
		std::this_thread::sleep_until(m_sleepUntil);
	}

	void setTargetFPS(int32 targetFPS)
	{
		m_targetFPS = targetFPS;
	}
};

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
	Addon::Register(U"FrameRateLimit", std::make_unique<FrameRateLimit>(300));

#ifdef _DEBUG
	// ライブラリ側のデバッグ用にコンソール表示(Debugビルドの場合のみ)
	Console.open();
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
