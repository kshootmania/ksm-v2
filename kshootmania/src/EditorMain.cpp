#include <Siv3D.hpp>
#include <CoTaskLib.hpp>
#include <NocoUI.hpp>
#include "Common/FrameRateLimit.hpp"
#include "Common/IMEUtils.hpp"
#include "Common/AssetManagement.hpp"
#include "Addon/AutoMuteAddon.hpp"
#include "Addon/CommonSEAddon.hpp"
#include "ksmaudio/ksmaudio.hpp"
#include "RuntimeConfig.hpp"
#include "Input/KeyConfig.hpp"

#ifdef __APPLE__
#include <ksmplatform_macos/input_method.h>
#endif

class Editor
{
private:
	bool m_exitRequested = false;

	std::shared_ptr<noco::Canvas> m_canvas;
	std::shared_ptr<noco::Canvas> m_editorCanvas;

	Size m_prevSceneSize;

public:
	Editor()
		: m_canvas(noco::Canvas::Create())
		, m_editorCanvas(noco::Canvas::Create(Scene::Size())->setAutoFitMode(noco::AutoFitMode::MatchSize))
		, m_prevSceneSize(Scene::Size())
	{
		// とりあえず何か出しとく
		auto messageNode = m_editorCanvas->emplaceChild(U"MessageNode", noco::InlineRegion{ .sizeRatio = Vec2::One() });
		messageNode->emplaceComponent<noco::Label>(U"Editorは未実装")
			->setFontSize(32)
			->setHorizontalAlign(noco::HorizontalAlign::Center)
			->setVerticalAlign(noco::VerticalAlign::Middle);
		const double screenWidth = static_cast<double>(Scene::Size().x);
		messageNode->emplaceComponent<noco::Tween>()
			->setTranslateEnabled(true)
			->setTranslateFrom(Vec2{ screenWidth, 0.0 })
			->setTranslateTo(Vec2{ -screenWidth, 0.0 })
			->setEasing(noco::TweenEasing::Linear)
			->setDuration(5.0)
			->setLoopType(noco::TweenLoopType::Loop);
	}

	void update()
	{
		m_editorCanvas->update();
		m_canvas->update();

		const auto sceneSize = Scene::Size();
		if (m_prevSceneSize != sceneSize)
		{
			m_prevSceneSize = sceneSize;
		}

		// ショートカットキー
		const bool isWindowActive = Window::GetState().focused;
		if (isWindowActive)
		{
			const bool ctrl = noco::detail::KeyCommandControl.pressed();
			const bool alt = KeyAlt.pressed();
			const bool shift = KeyShift.pressed();

#ifdef __APPLE__
			// macOSはCommand+Qで終了
			if (ctrl && !alt && !shift && KeyQ.down())
			{
				requestExit();
			}
#else
			// Windows/LinuxはCtrl+Wで終了
			if (ctrl && !alt && !shift && KeyW.down())
			{
				requestExit();
			}
#endif
		}
	}

	void draw() const
	{
		m_canvas->draw();
		m_editorCanvas->draw();
	}

	[[nodiscard]]
	bool isExitRequested() const noexcept
	{
		return m_exitRequested;
	}

	void requestExit() noexcept
	{
		m_exitRequested = true;
	}
};

void CreateAppDataDirectory()
{
#ifdef __APPLE__
	const FilePath appDataDir = FsUtils::AppDataDirectoryPath();
	Logger << U"[ksmeditor info] AppDataDirectory path: " << appDataDir;

	if (!FileSystem::Exists(appDataDir))
	{
		Logger << U"[ksmeditor info] Creating AppDataDirectory...";
		FileSystem::CreateDirectories(appDataDir);
		const bool created = FileSystem::Exists(appDataDir);
		Logger << U"[ksmeditor info] Created: " << created;
	}
	else
	{
		Logger << U"[ksmeditor info] AppDataDirectory already exists";
	}
#endif
}

void EditorMain()
{
	// Escキーによるプログラム終了を無効化
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	// F1キーによるライセンス表示を無効化
	LicenseManager::DisableDefaultTrigger();

	Window::SetTitle(U"K-Shoot Editor");
	Window::SetStyle(WindowStyle::Sizable);
	Window::Resize(800, 600);

	// カレントディレクトリを設定(ここ以外で変更しないこと)
	FileSystem::ChangeCurrentDirectory(FsUtils::ResourceDirectoryPath());

	// デフォルト色を指定
	Scene::SetBackground(Color{ 51, 51, 77 });
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

	// config.iniを読み込み
	ConfigIni::Load();

	// 言語ファイルを読み込み
	I18n::LoadLanguage(ConfigIni::GetString(ConfigIni::Key::kLanguage));

	// アセット一覧を登録
	AssetManagement::RegisterAssets();

	// フレームレート制限
	Graphics::SetVSyncEnabled(false);
	Addon::Register(U"FrameRateLimit", std::make_unique<FrameRateLimit>(300), -100);

	// 毎フレーム連続してアセット生成した時の警告を無効化
	Profiler::EnableAssetCreationWarning(false);

#ifdef _DEBUG
	// ライブラリ側のデバッグ用にコンソール表示(Debugビルドの場合のみ)
	Console.open();
#endif

	// ライブラリ初期化
	Co::Init();
	noco::Init();

	// NocoUIのグローバルデフォルトフォントを設定
	const Font uiFont = Font(FontMethod::MSDF, 36, Typeface::Regular);
	noco::SetGlobalDefaultFont(uiFont);

	Editor editor;

	while (System::Update())
	{
		editor.update();
		if (editor.isExitRequested())
		{
			break;
		}
		editor.draw();
	}

	// config.iniを保存
	ConfigIni::Save();

	// 音声のバックエンドを終了
	ksmaudio::Terminate();
}

void Main()
{
	try
	{
		EditorMain();
	}
	catch (const Error& e)
	{
		System::MessageBoxOK(e.what(), MessageBoxStyle::Error);
		throw;
	}
}
