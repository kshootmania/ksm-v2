#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include "scene/title/title_scene.hpp"
#include "scene/option/option_scene.hpp"
#include "scene/select/select_scene.hpp"
#include "scene/play/play_scene.hpp"
#include "ksmaudio/ksmaudio.hpp"

namespace
{
	constexpr double kSecToMs = 1000.0;
	constexpr double kMsToNs = 1000.0 * 1000.0;
	constexpr auto kMaxDrift = 10ms;

	constexpr double OneFrameMs(int32 fps)
	{
		return kSecToMs / fps;
	}

	constexpr double DeltaMs(auto now, auto prevTime)
	{
		const double ns = static_cast<double>((now - prevTime).count());
		return ns / kMsToNs;
	}
}

class FrameRateLimit : public IAddon
{
private:
	int32 m_targetFPS;

	std::optional<std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::nano>>> m_sleepUntil;

	void outputLog(double sleepDurationMs) const
	{
		const int32 fps = Profiler::FPS();
		const double deltaTimeMs = Scene::DeltaTime() * kSecToMs;
		Print << U"{}fps (sleepDt:{:.2f}ms, actualDt:{:.2f}ms)"_fmt(fps, sleepDurationMs, deltaTimeMs);
	}

public:
	explicit FrameRateLimit(int32 targetFPS)
		: m_targetFPS(targetFPS)
	{
	}

	virtual bool update() override // 本来はSystem::Update内のRendererのpresentの後ろで待つべき
	{
		if (!m_sleepUntil.has_value())
		{
			m_sleepUntil = std::chrono::steady_clock::now();
			return true;
		}

		// TODO: doubleを介さずにdurationのまま計算する
		*m_sleepUntil += std::chrono::duration<double, std::milli>(::OneFrameMs(m_targetFPS));

		// 目標フレームレートに届かなかったときにm_sleepUntilが現在時間より過去に離れていかないよう最小値でおさえる
		const auto sleepUntilMin = std::chrono::steady_clock::now() - kMaxDrift;
		if (*m_sleepUntil < sleepUntilMin)
		{
			*m_sleepUntil = sleepUntilMin;
		}

		//outputLog(DeltaMs(*m_sleepUntil, std::chrono::steady_clock::now()));

		std::this_thread::sleep_until(*m_sleepUntil);

		return true;
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

	Window::Resize(1024, 768);

	Graphics::SetVSyncEnabled(false);

#if defined(_WIN32) && defined(_DEBUG)
	AllocConsole();
	FILE* fp = NULL;
	freopen_s(&fp, "CONOUT$", "w", stdout);
#endif

	Addon::Register(U"FrameRateLimit", std::make_unique<FrameRateLimit>(300));

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
		}
	}

	// config.iniを保存
	ConfigIni::Save();

	// 音声のバックエンドを終了
	ksmaudio::Terminate();
}
