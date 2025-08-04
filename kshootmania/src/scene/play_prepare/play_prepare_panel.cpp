#include "play_prepare_panel.hpp"
#include "music_game/graphics/graphics_defines.hpp"
#include "graphics/font_utils.hpp"



namespace
{
	constexpr StringView kTitlePanelBaseTextureFilename = U"pl_minfo.png";
	constexpr Size kTitlePanelSize = Size{ 720, 180 };

	constexpr StringView kNumberTextureFontFilename = U"num2.png";

	constexpr StringView kNumberLargeTextureFontFilename = U"num2_large.png";

	constexpr StringView kDifficultyTextureFilename = U"pl_difficulty.png";

	constexpr Point kTitlePanelBasePosition = { 0 , 120 };

	RenderTexture CreateRenderTexture(FilePathView chartFilePath, const kson::ChartData& chartData)
	{
		const Texture panelTextureAsset = TextureAsset(kTitlePanelBaseTextureFilename);

		RenderTexture renderTexture(panelTextureAsset.size());
		Shader::Copy(panelTextureAsset, renderTexture);

		// 曲名・アーティスト名のパネルのテクスチャははじめに用意しておく
		{
			const ScopedRenderTarget2D renderTarget(renderTexture);
			const ScopedRenderStates2D renderState(SamplerState::ClampAniso);

			// Title
			const Font titleFont = AssetManagement::SystemFontBold();
			FontUtils::DrawTextCenterWithFitWidth(titleFont(Unicode::FromUTF8(chartData.meta.title)), 27, 25, { 150, 28, 458, 53 }, Palette::Black);

			// Artist
			const Font artistFont = AssetManagement::SystemFont();
			FontUtils::DrawTextCenterWithFitWidth(artistFont(Unicode::FromUTF8(chartData.meta.artist)), 20, 19, { 150, 48, 458, 73 }, Palette::Black);
		}

		return renderTexture;
	}
}

PlayPreparePanel::PlayPreparePanel(FilePathView chartFilePath, const kson::ChartData& chartData)
	: m_titlePanelBaseTexture(CreateRenderTexture(chartFilePath, chartData))
	, m_titlePanelPosition(Scene::Width() / 2 + Scaled(kTitlePanelBasePosition.x), Scene::Height() / 2 + Scaled(kTitlePanelBasePosition.y))
	, m_difficultyTexture(kDifficultyTextureFilename,
		{
			.row = kNumDifficulties,
			.sourceScale = SourceScale::k2x,
			.sourceSize = { 126, 144 / 4 }
		})
	, m_difficultyTextureRegion(m_difficultyTexture(chartData.meta.difficulty.idx))
	, m_level(chartData.meta.level)
	, m_bpm(chartData.meta.stdBPM ? chartData.meta.stdBPM : chartData.beat.bpm.at(0))
	, m_numberTextureFont(kNumberTextureFontFilename, { 40 / 2, 228 / 16 })
	, m_numberLargeTextureFont(kNumberLargeTextureFontFilename, { 60 / 2, 432 / 16 })
	, m_levelNumberLayout(Scaled(20, 18), TextureFontTextLayout::Align::Left)
	, m_bpmNumberLayout(Scaled(20, 18), TextureFontTextLayout::Align::Left)
{
}

void PlayPreparePanel::draw() const
{
	m_titlePanelBaseTexture.resized(ScaledL(kTitlePanelSize * 2)).drawAt(m_titlePanelPosition);
	m_difficultyTextureRegion.draw(m_titlePanelPosition + Scaled(-150, 32));

	// Level
	m_numberLargeTextureFont.draw(m_levelNumberLayout, m_titlePanelPosition + Scaled(-50, 31), m_level, ZeroPaddingYN::No);

	// BPM
	m_numberLargeTextureFont.draw(m_bpmNumberLayout, m_titlePanelPosition + Scaled(100, 31), static_cast<int32>(round(m_bpm)), ZeroPaddingYN::No);
}
