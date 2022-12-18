#include "asset_management.hpp"

namespace
{
	void RegisterTextureAssets()
	{
		// "imgs"ディレクトリ以下の全ファイルを登録
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
		// "se"ディレクトリ以下の全ファイルを登録
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
	// 注意: アセットの実際のロードはここではなく、各シーンでの使用時に実施される
	RegisterTextureAssets();
	RegisterAudioAssets();
}
