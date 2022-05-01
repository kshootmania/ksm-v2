#include "asset_management.hpp"

namespace
{
	void RegisterTextureAssets()
	{
		// Register all files under the "imgs" directory
		constexpr FilePathView kImgPath = U"imgs";
		const FilePath imgFullPath = FileSystem::FullPath(kImgPath);
		for (const FilePath& path : FileSystem::DirectoryContents(imgFullPath, Recursive::Yes))
		{
			TextureAsset::Register(FileSystem::RelativePath(path, imgFullPath), path);
			TextureAsset::Register(kAssetPrefixSRGB + FileSystem::RelativePath(path, imgFullPath), path, TextureDesc::UnmippedSRGB);
		}
	}

	void RegisterAudioAssets()
	{
		// Register all files under the "se" directory
		constexpr FilePathView kSePath = U"se";
		const FilePath seFullPath = FileSystem::FullPath(kSePath);
		for (const FilePath& path : FileSystem::DirectoryContents(seFullPath, Recursive::Yes))
		{
			AudioAsset::Register(FileSystem::RelativePath(path, seFullPath), path);
		}

		// TODO: BGM assets
	}
}

void AssetManagement::RegisterAssets()
{
	// Note: Assets are not loaded here. They are loaded at the time of use.
	RegisterTextureAssets();
	RegisterAudioAssets();
}
