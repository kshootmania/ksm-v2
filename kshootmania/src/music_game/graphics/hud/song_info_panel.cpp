#include "song_info_panel.hpp"
#include "music_game/graphics/graphics_defines.hpp"
#include "graphics/font_utils.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kTitlePanelBaseTextureFilename = U"minfo_label.png";
		constexpr Size kTitlePanelSize = { 240, 44 };

		constexpr StringView kDetailPanelBaseTextureFilename = U"minfo_detail.png";
		constexpr Size kDetailPanelSize = { 240, 66 };

		constexpr StringView kNumberTextureFontFilename = U"num2.png";

		constexpr StringView kDifficultyTextureFilename = U"result_difficulty.png";

		constexpr double kJacketWidth = 38.5;
		constexpr Vec2 kJacketPosition = { -286.5, 45.5 };
		constexpr Point kTitlePanelBasePosition = { -155, 45 };
		constexpr Point kDetailPanelBasePosition = { -295, 69 };
	}

	SongInfoPanel::SongInfoPanel(const kson::ChartData& chartData, FilePathView parentPath)
		: m_jacketTexture(parentPath + Unicode::FromUTF8(chartData.meta.jacketFilename))
		, m_scaledJacketSize(RectSizeInSquare(m_jacketTexture.size(), Scaled(kJacketWidth) ))
		, m_titlePanelBaseTexture(kTitlePanelSize * 2, kTransparent)
		, m_detailPanelBaseTexture(TextureAsset(kDetailPanelBaseTextureFilename))
		, m_difficultyTexture(kDifficultyTextureFilename,
			{
				.row = kNumDifficulties,
				.sourceScale = SourceScale::k2x,
				.sourceSize = { 84, 96 / 4 },
			})
		, m_difficultyTextureRegion(m_difficultyTexture(chartData.meta.difficulty.idx))
		, m_level(chartData.meta.level)
		, m_numberTextureFont(kNumberTextureFontFilename, { 40 / 2, 288 / 16 })
		, m_levelNumberLayout(Scaled(10, 9), TextureFontTextLayout::Align::Left)
		, m_bpmNumberLayout(Scaled(10, 9), TextureFontTextLayout::Align::Left)
	{
		// 曲名・アーティスト名のパネルのテクスチャははじめに用意しておく
		{
			Shader::Copy(TextureAsset(kTitlePanelBaseTextureFilename), m_titlePanelBaseTexture);

			ScopedRenderTarget2D renderTarget(m_titlePanelBaseTexture);
			ScopedRenderStates2D blendState(kEnableAlphaBlend); // Note: The drawn text edge will be darker than expected, but no problem here.

			// Title
			const Font titleFont = AssetManagement::SystemFontBold();
			FontUtils::DrawTextCenterWithFitWidth(titleFont(Unicode::FromUTF8(chartData.meta.title)), 27, 25, { 36, 5, 408, 50 }, kSongInfoFontColor);

			// Artist
			const Font artistFont = AssetManagement::SystemFont();
			FontUtils::DrawTextCenterWithFitWidth(artistFont(Unicode::FromUTF8(chartData.meta.artist)), 22, 21, { 36, 54, 408, 28 }, kSongInfoFontColor);
		}
	}

	void SongInfoPanel::draw(double currentBPM, const Scroll::HighwayScrollContext& highwayScrollContext) const
	{
		const Vec2 jacketPosition(Scene::Width() / 2 + static_cast<int32>(Scaled(kJacketPosition.x)), static_cast<int32>(Scaled(kJacketPosition.y)));
		const Vec2 titlePanelPosition(Scene::Width() / 2 + Scaled(kTitlePanelBasePosition.x), Scaled(kTitlePanelBasePosition.y));
		const Vec2 detailPanelPosition(Scene::Width() / 2 + Scaled(kDetailPanelBasePosition.x), Scaled(kDetailPanelBasePosition.y));
		
		m_jacketTexture.resized(m_scaledJacketSize).drawAt(jacketPosition);
		m_titlePanelBaseTexture.resized(Scaled(kTitlePanelSize)).drawAt(titlePanelPosition);
		m_detailPanelBaseTexture.resized(Scaled(kDetailPanelSize)).draw(detailPanelPosition);
		m_difficultyTextureRegion.draw(detailPanelPosition + Scaled(13, 3));

		// Level
		m_numberTextureFont.draw(m_levelNumberLayout, detailPanelPosition + Scaled(79, 4), m_level, ZeroPaddingYN::No);

		// BPM
		m_numberTextureFont.draw(m_bpmNumberLayout, detailPanelPosition + Scaled(159, 4), static_cast<int32>(round(currentBPM)), ZeroPaddingYN::No);

		// ハイスピード設定
		m_hispeedSettingPanel.draw(detailPanelPosition + Scaled(159, 27), highwayScrollContext);
	}
}
