#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include "scene/title/title_assets.hpp"
#include "scene/option/option_assets.hpp"
#include "i18n/i18n.hpp"
#include "ini/config_ini.hpp"

void RegisterAssets()
{
	// Note: Assets are not loaded here. They are loaded at the time of use.

	// Register all files under the "imgs" directory
	constexpr FilePathView kImgPath = U"imgs";
	const FilePath imgFullPath = FileSystem::FullPath(kImgPath);
	for (const FilePath& path : FileSystem::DirectoryContents(imgFullPath, Recursive::Yes))
	{
		TextureAsset::Register(FileSystem::RelativePath(path, imgFullPath), path);
	}

	// Register all files under the "se" directory
	constexpr FilePathView kSePath = U"se";
	const FilePath seFullPath = FileSystem::FullPath(kSePath);
	for (const FilePath& path : FileSystem::DirectoryContents(seFullPath, Recursive::Yes))
	{
		AudioAsset::Register(FileSystem::RelativePath(path, seFullPath), path);
	}
}

void Main()
{
	// Disable application termination by Esc key
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	// Load language text file
	I18n::LoadLanguage(U"Japanese");

	// Load config.ini
	ConfigIni::Load();

	// Register asset list
	RegisterAssets();

	// Main loop
	SceneManager<StringView> sceneManager = SceneManagement::MakeSceneManager();
	while (System::Update())
	{
		if (!sceneManager.update())
		{
			break;
		}
	}

	// Save config.ini
	ConfigIni::Save();
}
