#include "song_info_panel.hpp"
#include "music_game/graphics/graphics_defines.hpp"

namespace
{
	constexpr StringView kTitlePanelBaseTextureFilename = U"minfo_label.png";
	constexpr Size kTitlePanelSize = { 480, 88 };

	constexpr StringView kDetailPanelBaseTextureFilename = U"minfo_detail.png";
	constexpr Size kDetailPanelSize = { 240, 66 };

	constexpr StringView kNumberFontTextureFilename = U"num2.png";

	constexpr double kJacketWidth = 38.5;
	constexpr Vec2 kJacketPosition = { -286.5, 45.5 };
	constexpr Point kTitlePanelBasePosition = { -155, 45 };
	constexpr Point kDetailPanelBasePosition = { -295, 69 };

	SizeF ScaledJacketSize(Size sourceSize)
	{
		SizeF size = ScreenUtils::Scaled(SizeF{ kJacketWidth, kJacketWidth });
		if (sourceSize.x < sourceSize.y)
		{
			size.x *= static_cast<double>(sourceSize.x) / sourceSize.y;
		}
		else if (sourceSize.y < sourceSize.x)
		{
			size.y *= static_cast<double>(sourceSize.y) / sourceSize.x;
		}
		return size;
	}
}

MusicGame::Graphics::SongInfoPanel::SongInfoPanel(const ksh::ChartData& chartData)
	: m_jacketTexture(FileSystem::ParentPath(Unicode::Widen(chartData.filePath)) + Unicode::FromUTF8(chartData.meta.jacketFilename))
	, m_scaledJacketSize(ScaledJacketSize(m_jacketTexture.size()))
	, m_titlePanelBaseTexture(kTitlePanelSize, kTransparent)
	, m_detailPanelBaseTexture(ScreenUtils::Scaled(kDetailPanelSize), kTransparent)
	, m_numberFontTexture(kNumberFontTextureFilename, ScreenUtils::Scaled(10, 9), { 20, 18 })
{
	using namespace ScreenUtils;

	// Predraw song title panel texture
	{
		Shader::Copy(TextureAsset(kTitlePanelBaseTextureFilename), m_titlePanelBaseTexture);

		ScopedRenderTarget2D renderTarget(m_titlePanelBaseTexture);
		ScopedRenderStates2D blendState(kEnableAlphaBlend);

		// Title
		const Font titleFont(30, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default);
		const auto drawableTitle = titleFont(Unicode::FromUTF8(chartData.meta.title));
		for (int32 i = 0; i < 2; ++i)
		{
			// Draw twice to make the font slightly bold
			// (Using a bold typeface makes the font too bold compared to HSP's Artlet2D)
			// TODO: Maybe fonts with bold glyphs (e.g. Arial) do not have this problem (unconfirmed), so simply use bold for them.
			drawableTitle.drawAt(Point{ 12 + i, 4 } + Point{ 448, 52 } / 2, kSongInfoFontColor);
		}

		// Artist
		const Font artistFont(22, FileSystem::GetFolderPath(SpecialFolder::SystemFonts) + U"msgothic.ttc", 2, FontStyle::Default);
		artistFont(Unicode::FromUTF8(chartData.meta.artist)).drawAt(Point{ 12, 54 } + Point{ 448, 28 } / 2, kSongInfoFontColor);
	}

	// Predraw detail panel (difficulty/level/BPM/cursor) texture
	{
		ScopedRenderStates2D blendState(BlendState::Opaque);
		ScopedRenderTarget2D renderTarget(m_detailPanelBaseTexture);
		TextureAsset(kDetailPanelBaseTextureFilename).resized(Scaled(kDetailPanelSize)).draw();
	}
}

void MusicGame::Graphics::SongInfoPanel::draw() const
{
	using namespace ScreenUtils;

	m_jacketTexture.resized(m_scaledJacketSize).drawAt(Scene::Width() / 2 + static_cast<int32>(Scaled(kJacketPosition.x)), static_cast<int32>(Scaled(kJacketPosition.y)));
	m_titlePanelBaseTexture.resized(Scaled2x(kTitlePanelSize)).drawAt(Scene::Width() / 2 + Scaled(kTitlePanelBasePosition.x), Scaled(kTitlePanelBasePosition.y));
	m_detailPanelBaseTexture.draw(Scene::Width() / 2 + Scaled(kDetailPanelBasePosition.x), Scaled(kDetailPanelBasePosition.y));
}
