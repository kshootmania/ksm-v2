#include "asset_management.hpp"

namespace AssetManagement
{
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
		}

		void RegisterFontAssets()
		{
			// フォントを登録
			FontAsset::Register(kFontAssetSystem, FontMethod::SDF, 600, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default);
			FontAsset::Register(kFontAssetSystemBold, FontMethod::SDF, 600, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Bold);
		}
	}

	void RegisterAssets()
	{
		// 注意: アセットの実際のロードはここではなく、各シーンでの使用時に実施される
		RegisterTextureAssets();
		RegisterAudioAssets();
		RegisterFontAssets();
	}
}
