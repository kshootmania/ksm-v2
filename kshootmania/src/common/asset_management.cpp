#include "asset_management.hpp"

namespace AssetManagement
{
	namespace
	{
		constexpr StringView kFontAssetSystem = U"System";
		constexpr StringView kFontAssetSystemBold = U"SystemBold";
		constexpr StringView kFontAssetSystemJp = U"SystemJp";
		constexpr StringView kFontAssetSystemJpBold = U"SystemJpBold";
		constexpr StringView kFontAssetSystemJpCJK = U"SystemJpCJK";
		constexpr StringView kFontAssetSystemJpCJKBold = U"SystemJpCJKBold";
		constexpr StringView kFontAssetSystemKr = U"SystemKr";
		constexpr StringView kFontAssetSystemKrBold = U"SystemKrBold";
		constexpr StringView kFontAssetSystemKrCJK = U"SystemKrCJK";
		constexpr StringView kFontAssetSystemKrCJKBold = U"SystemKrCJKBold";
		constexpr StringView kFontAssetSystemSc = U"SystemSc";
		constexpr StringView kFontAssetSystemScBold = U"SystemScBold";
		constexpr StringView kFontAssetSystemTc = U"SystemTc";
		constexpr StringView kFontAssetSystemTcBold = U"SystemTcBold";
		constexpr StringView kFontAssetSystemEmoji = U"SystemEmoji";
		constexpr StringView kFontAssetSystemEmojiBold = U"SystemEmojiBold";
		constexpr StringView kFontResourcePath = U"assets/font/tektur-ksm/Tektur-KSM-Medium.ttf";
		constexpr StringView kFontJaResourcePath = U"assets/font/corporate-logo/Corporate-Logo-Medium-ver3.otf";
		constexpr StringView kFontKrResourcePath = U"assets/font/noto-sans-kr/NotoSansKR-Medium.ttf"; // CJKフォントだとハングルが細かったので別途用意

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
			FontAsset::Register(kFontAssetSystem, FontMethod::MSDF, 44, Resource(kFontResourcePath), FontStyle::Default);
			FontAsset::Register(kFontAssetSystemBold, FontMethod::MSDF, 44, Resource(kFontResourcePath), FontStyle::Bold);
			FontAsset::Register(kFontAssetSystemJp, FontMethod::MSDF, 44, Resource(kFontJaResourcePath), FontStyle::Default);
			FontAsset::Register(kFontAssetSystemJpBold, FontMethod::MSDF, 44, Resource(kFontJaResourcePath), FontStyle::Bold);
			FontAsset::Register(kFontAssetSystemJpCJK, FontMethod::MSDF, 44, Typeface::CJK_Regular_JP, FontStyle::Default);
			FontAsset::Register(kFontAssetSystemJpCJKBold, FontMethod::MSDF, 44, Typeface::CJK_Regular_JP, FontStyle::Bold);
			FontAsset::Register(kFontAssetSystemKr, FontMethod::MSDF, 44, Resource(kFontKrResourcePath), FontStyle::Default);
			FontAsset::Register(kFontAssetSystemKrBold, FontMethod::MSDF, 44, Resource(kFontKrResourcePath), FontStyle::Bold);
			FontAsset::Register(kFontAssetSystemKrCJK, FontMethod::MSDF, 44, Typeface::CJK_Regular_KR, FontStyle::Default);
			FontAsset::Register(kFontAssetSystemKrCJKBold, FontMethod::MSDF, 44, Typeface::CJK_Regular_KR, FontStyle::Bold);
			FontAsset::Register(kFontAssetSystemSc, FontMethod::MSDF, 44, Typeface::CJK_Regular_SC);
			FontAsset::Register(kFontAssetSystemScBold, FontMethod::MSDF, 44, Typeface::CJK_Regular_SC, FontStyle::Bold);
			FontAsset::Register(kFontAssetSystemTc, FontMethod::MSDF, 44, Typeface::CJK_Regular_TC);
			FontAsset::Register(kFontAssetSystemTcBold, FontMethod::MSDF, 44, Typeface::CJK_Regular_TC, FontStyle::Bold);
			FontAsset::Register(kFontAssetSystemEmoji, FontMethod::MSDF, 44, Typeface::MonochromeEmoji);
			FontAsset::Register(kFontAssetSystemEmojiBold, FontMethod::MSDF, 44, Typeface::MonochromeEmoji, FontStyle::Bold);
		}

		Font MakeFallbackFont(StringView fontAssetName1, StringView fontAssetName2, StringView fontAssetName3, StringView fontAssetName4 = U"", StringView fontAssetName5 = U"")
		{
			const Font font1 = FontAsset(fontAssetName1);
			const Font font2 = FontAsset(fontAssetName2);
			font1.addFallback(font2);
			const Font font3 = FontAsset(fontAssetName3);
			font1.addFallback(font3);
			if (!fontAssetName4.empty())
			{
				const Font font4 = FontAsset(fontAssetName4);
				font1.addFallback(font4);
			}
			if (!fontAssetName5.empty())
			{
				const Font font5 = FontAsset(fontAssetName5);
				font1.addFallback(font5);
			}
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
			// 簡体字の場合は簡体字デザインのCJKフォントを優先
			return MakeFallbackFont(kFontAssetSystem, kFontAssetSystemSc, kFontAssetSystemEmoji);

		case I18n::StandardLanguage::TraditionalChinese:
			// 繁体字の場合は繁体字デザインのCJKフォントを優先
			return MakeFallbackFont(kFontAssetSystem, kFontAssetSystemTc, kFontAssetSystemEmoji);

		case I18n::StandardLanguage::Korean:
			// 韓国語の場合は日本語フォントを優先し、韓国語デザインのCJKフォントをフォールバック指定
			// (日本語フォントにハングルは含まれないため日本語フォントを優先してもさほど問題ない)
			return MakeFallbackFont(kFontAssetSystem, kFontAssetSystemJp, kFontAssetSystemKr, kFontAssetSystemKrCJK, kFontAssetSystemEmoji);

		default:
			// それ以外の場合は日本語フォントを優先
			return MakeFallbackFont(kFontAssetSystem, kFontAssetSystemJp, kFontAssetSystemJpCJK, kFontAssetSystemEmoji);
		}
	}

	Font SystemFontBold()
	{
		const I18n::StandardLanguage language = I18n::CurrentLanguage();
		switch (language)
		{
		case I18n::StandardLanguage::SimplifiedChinese:
			// 簡体字の場合は簡体字デザインのCJKフォントを優先
			return MakeFallbackFont(kFontAssetSystemBold, kFontAssetSystemScBold, kFontAssetSystemEmojiBold);

		case I18n::StandardLanguage::TraditionalChinese:
			// 繁体字の場合は繁体字デザインのCJKフォントを優先
			return MakeFallbackFont(kFontAssetSystemBold, kFontAssetSystemTcBold, kFontAssetSystemEmojiBold);

		case I18n::StandardLanguage::Korean:
			// 韓国語の場合は日本語フォントを優先し、韓国語デザインのCJKフォントをフォールバック指定
			// (日本語フォントにハングルは含まれないため日本語フォントを優先してもさほど問題ない)
			return MakeFallbackFont(kFontAssetSystemBold, kFontAssetSystemJpBold, kFontAssetSystemKrBold, kFontAssetSystemKrCJK, kFontAssetSystemEmojiBold);

		default:
			// それ以外の場合は日本語フォントを優先
			return MakeFallbackFont(kFontAssetSystemBold, kFontAssetSystemJpBold, kFontAssetSystemJpCJKBold, kFontAssetSystemEmojiBold);
		}
	}
}
