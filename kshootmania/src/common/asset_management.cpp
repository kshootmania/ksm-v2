#include "asset_management.hpp"

namespace AssetManagement
{
	namespace
	{
		constexpr StringView kFontAssetSystem = U"System";
		constexpr StringView kFontAssetSystemBold = U"SystemBold";
		constexpr StringView kFontAssetSystemKr = U"SystemKr";
		constexpr StringView kFontAssetSystemKrBold = U"SystemKrBold";
		constexpr StringView kFontAssetSystemSc = U"SystemSc";
		constexpr StringView kFontAssetSystemScBold = U"SystemScBold";
		constexpr StringView kFontAssetSystemTc = U"SystemTc";
		constexpr StringView kFontAssetSystemTcBold = U"SystemTcBold";
		constexpr StringView kDefaultFontResourcePath = U"assets/font/corporate-logo/Corporate-Logo-ver3-ksm.otf";

		void RegisterTextureAssets()
		{
			// "imgs"ディレクトリ以下の全ファイルを登録
			constexpr FilePathView kImgPath = U"imgs";
			const FilePath imgFullPath = FileSystem::FullPath(kImgPath);
			for (const FilePath& path : FileSystem::DirectoryContents(imgFullPath, Recursive::Yes))
			{
				TextureAsset::Register(FileSystem::RelativePath(path, imgFullPath), path);
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
			FontAsset::Register(kFontAssetSystem, FontMethod::MSDF, 44, Resource(kDefaultFontResourcePath), 0, FontStyle::Default);
			FontAsset::Register(kFontAssetSystemBold, FontMethod::MSDF, 44, Resource(kDefaultFontResourcePath), 0, FontStyle::Bold);
			FontAsset::Register(kFontAssetSystemKr, FontMethod::MSDF, 44, Typeface::CJK_Regular_KR);
			FontAsset::Register(kFontAssetSystemKrBold, FontMethod::MSDF, 44, Typeface::CJK_Regular_KR, FontStyle::Bold);
			FontAsset::Register(kFontAssetSystemSc, FontMethod::MSDF, 44, Typeface::CJK_Regular_SC);
			FontAsset::Register(kFontAssetSystemScBold, FontMethod::MSDF, 44, Typeface::CJK_Regular_SC, FontStyle::Bold);
			FontAsset::Register(kFontAssetSystemTc, FontMethod::MSDF, 44, Typeface::CJK_Regular_TC);
			FontAsset::Register(kFontAssetSystemTcBold, FontMethod::MSDF, 44, Typeface::CJK_Regular_TC, FontStyle::Bold);
		}

		Font MakeFallbackFont(StringView fontAssetName1, StringView fontAssetName2)
		{
			const Font font1 = FontAsset(fontAssetName1);
			const Font font2 = FontAsset(fontAssetName2);
			font1.addFallback(font2);
			return font1;
		}
	}

	void RegisterAssets()
	{
		// 注意: アセットの実際のロードはここではなく、各シーンでの使用時に実施される
		RegisterTextureAssets();
		RegisterAudioAssets();
		RegisterFontAssets();
	}

	Font SystemFont()
	{
		const I18n::StandardLanguage language = I18n::CurrentLanguage();
		switch (language)
		{
		case I18n::StandardLanguage::SimplifiedChinese:
			// 簡体字の場合は簡体字デザインのCJKフォントを使用
			// (一部の漢字が日本語フォントで描画されるのを防ぐためフォールバック指定にしない)
			return FontAsset(kFontAssetSystemSc);

		case I18n::StandardLanguage::TraditionalChinese:
			// 繁体字の場合は繁体字デザインのCJKフォントを使用
			// (一部の漢字が日本語フォントで描画されるのを防ぐためフォールバック指定にしない)
			return FontAsset(kFontAssetSystemTc);

		default:
			// それ以外の場合は日本語フォントを優先し、韓国語デザインのCJKフォントをフォールバック指定(CJKフォントに中国語も含まれる)
			return MakeFallbackFont(kFontAssetSystem, kFontAssetSystemKr);
		}
	}

	Font SystemFontBold()
	{
		const I18n::StandardLanguage language = I18n::CurrentLanguage();
		switch (language)
		{
		case I18n::StandardLanguage::SimplifiedChinese:
			// 簡体字の場合は簡体字デザインのCJKフォントを使用
			// (一部の漢字が日本語フォントで描画されるのを防ぐためフォールバック指定にしない)
			return FontAsset(kFontAssetSystemScBold);

		case I18n::StandardLanguage::TraditionalChinese:
			// 繁体字の場合は繁体字デザインのCJKフォントを使用
			// (一部の漢字が日本語フォントで描画されるのを防ぐためフォールバック指定にしない)
			return FontAsset(kFontAssetSystemTcBold);

		default:
			// それ以外の場合は日本語フォントを優先し、韓国語デザインのCJKフォントをフォールバック指定(CJKフォントに中国語も含まれる)
			return MakeFallbackFont(kFontAssetSystemBold, kFontAssetSystemKrBold);
		}
	}
}
