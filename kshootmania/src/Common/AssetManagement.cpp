#include "AssetManagement.hpp"

namespace AssetManagement
{
	namespace
	{
		constexpr StringView kFontAssetSystem = U"System";
		constexpr StringView kFontAssetSystemBold = U"SystemBold";
		constexpr StringView kFontResourcePathJa = U"assets/font/KSM-JA-Medium.ttf";
		constexpr StringView kFontResourcePathSc = U"assets/font/KSM-SC-Medium.ttf";
		constexpr StringView kFontResourcePathTc = U"assets/font/KSM-TC-Medium.ttf";

		FilePath GetFontPathForCurrentLanguage()
		{
			const I18n::StandardLanguage language = I18n::CurrentLanguage();
			switch (language)
			{
			case I18n::StandardLanguage::SimplifiedChinese:
				return FilePath{ kFontResourcePathSc };
			case I18n::StandardLanguage::TraditionalChinese:
				return FilePath{ kFontResourcePathTc };
			default:
				return FilePath{ kFontResourcePathJa };
			}
		}

		void RegisterTextureAssets()
		{
			// "imgs"ディレクトリ以下の全ファイルを登録
			const FilePath imgFullPath = FsUtils::GetResourcePath(U"imgs");
			for (const FilePath& path : FileSystem::DirectoryContents(imgFullPath, Recursive::Yes))
			{
				TextureAsset::Register(FileSystem::RelativePath(path, imgFullPath), path);
			}
		}

		void RegisterAudioAssets()
		{
			// "se"ディレクトリ以下の全ファイルを登録
			const FilePath seFullPath = FsUtils::GetResourcePath(U"se");
			for (const FilePath& path : FileSystem::DirectoryContents(seFullPath, Recursive::Yes))
			{
				AudioAsset::Register(FileSystem::RelativePath(path, seFullPath), path);
			}
		}

		void RegisterFontAssets()
		{
			const FilePath fontPath = GetFontPathForCurrentLanguage();
			if (FontAsset::IsRegistered(kFontAssetSystem))
			{
				FontAsset::Unregister(kFontAssetSystem);
			}
			if (FontAsset::IsRegistered(kFontAssetSystemBold))
			{
				FontAsset::Unregister(kFontAssetSystemBold);
			}
#ifdef __linux
			constexpr StringView kResourceDir = U"resources/";
			FontAsset::Register(kFontAssetSystem, FontMethod::MSDF, 44, kResourceDir + fontPath, FontStyle::Default);
			FontAsset::Register(kFontAssetSystemBold, FontMethod::MSDF, 44, kResourceDir + fontPath, FontStyle::Bold);
#else
			FontAsset::Register(kFontAssetSystem, FontMethod::MSDF, 44, Resource(fontPath), FontStyle::Default);
			FontAsset::Register(kFontAssetSystemBold, FontMethod::MSDF, 44, Resource(fontPath), FontStyle::Bold);
#endif
		}
	}

	void RegisterAssets()
	{
		// 注意: アセットの実際のロードはここではなく、各シーンでの使用時に実施される
		RegisterTextureAssets();
		RegisterAudioAssets();
		// フォントは言語読み込み後にRefreshSystemFontForCurrentLanguage()で登録される
	}

	void RefreshSystemFontForCurrentLanguage()
	{
		if (FontAsset::IsRegistered(kFontAssetSystem))
		{
			FontAsset::Unregister(kFontAssetSystem);
			FontAsset::Unregister(kFontAssetSystemBold);
		}
		RegisterFontAssets();
	}

	Font SystemFont()
	{
		return FontAsset(kFontAssetSystem);
	}

	Font SystemFontBold()
	{
		return FontAsset(kFontAssetSystemBold);
	}
}
