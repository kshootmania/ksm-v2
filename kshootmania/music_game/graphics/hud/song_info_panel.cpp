#include "song_info_panel.hpp"
#include "music_game/graphics/graphics_defines.hpp"

namespace
{
	constexpr StringView kTitlePanelBaseTextureFilename = U"minfo_label.png";
	constexpr Size kTitlePanelSize = { 240, 44 };

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
	: m_jacketTexture(FileSystem::ParentPath(Unicode::Widen(chartData.filePath)) + Unicode::Widen(chartData.meta.jacketFilename))
	, m_scaledJacketSize(ScaledJacketSize(m_jacketTexture.size()))
	, m_titlePanelBaseTexture(ScreenUtils::Scaled(kTitlePanelSize), kTransparent)
	, m_detailPanelBaseTexture(ScreenUtils::Scaled(kDetailPanelSize), kTransparent)
	, m_numberFontTexture(kNumberFontTextureFilename, ScreenUtils::Scaled(10, 9), { 20, 18 })
{
	// Predraw base textures
	ScopedRenderStates2D blendState(kEnableAlphaBlend);
	{
		ScopedRenderTarget2D renderTarget(m_titlePanelBaseTexture);
		TextureAsset(kTitlePanelBaseTextureFilename).resized(ScreenUtils::Scaled(kTitlePanelSize)).draw();
	}
	{
		ScopedRenderTarget2D renderTarget(m_detailPanelBaseTexture);
		TextureAsset(kDetailPanelBaseTextureFilename).resized(ScreenUtils::Scaled(kDetailPanelSize)).draw();
	}
}

void MusicGame::Graphics::SongInfoPanel::draw() const
{
	using namespace ScreenUtils;

	m_jacketTexture.resized(m_scaledJacketSize).drawAt(Scene::Width() / 2 + static_cast<int32>(Scaled(kJacketPosition.x)), static_cast<int32>(Scaled(kJacketPosition.y)));
	m_titlePanelBaseTexture.drawAt(Scene::Width() / 2 + Scaled(kTitlePanelBasePosition.x), Scaled(kTitlePanelBasePosition.y));
	m_detailPanelBaseTexture.draw(Scene::Width() / 2 + Scaled(kDetailPanelBasePosition.x), Scaled(kDetailPanelBasePosition.y));
}
