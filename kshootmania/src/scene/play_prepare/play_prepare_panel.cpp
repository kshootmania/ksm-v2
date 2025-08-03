#include "play_prepare_panel.hpp"
#include "graphics/number_texture_font.hpp"
#include "graphics/font_utils.hpp"

namespace
{
	constexpr Size kInfoSize = Size{ 720, 180 } * 2;

	constexpr Size kHispeedSize = Size{ 452, 54 } * 2;

	RenderTexture CreateRenderTexture(FilePathView chartFilePath, const kson::ChartData& chartData)
	{
		using namespace MusicGame;

		const Texture panelTextureAsset = TextureAsset(PlayPrepareTexture::kInfo);
		const NumberTextureFont bpmFont(PlayPrepareTexture::kBPMFont, { 30 , 27 });
		const NumberTextureFont levelFont(PlayPrepareTexture::kLevelFont, { 30 , 27 });
		const NumberTextureFont hsTypeFont(PlayPrepareTexture::kHiSpeedTypeFont, { 20 , 18 });
		const NumberTextureFont hispeedFont(PlayPrepareTexture::kHispeedFont, { 20 , 18 });

		const Size panelSize = panelTextureAsset.size();
		RenderTexture renderTexture(panelSize);
		Shader::Copy(panelTextureAsset, renderTexture);

		const ScopedRenderTarget2D renderTarget(renderTexture);

		// 曲名を表示
		const Font fontBold = AssetManagement::SystemFontBold();
		FontUtils::DrawTextCenterWithFitWidth(fontBold(Unicode::FromUTF8(chartData.meta.title)), 27, 25, { 150, 28, 458, 53 }, Palette::Black);

		//// アーティストを表示
		const Font font = AssetManagement::SystemFont();
		FontUtils::DrawTextCenterWithFitWidth(font(Unicode::FromUTF8(chartData.meta.artist)), 20, 19, { 150, 48, 458, 73 }, Palette::Black);

		//// 難易度を表示
		//const TextureFontTextLayout levelTextLayout({ 30, 27 }, TextureFontTextLayout::Align::Left, 8, 50.0);
		//bpmFont.draw(levelTextLayout, { 100, 400 }, chartData.meta.level, ZeroPaddingYN::Yes);

		//// BPM
		//const TiledTexture BPMTiledTexture(PlayPrepareTexture::kDifficulty, TiledTextureSizeInfo{
		//	.row = kNumDifficulties,
		//	.sourceSize = { 126, 36 }
		//});

		return renderTexture;
	}
}

PlayPreparePanel::PlayPreparePanel(FilePathView chartFilePath, const kson::ChartData& chartData)
	: m_infoRenderTexture(CreateRenderTexture(chartFilePath, chartData))
{
}

void PlayPreparePanel::draw() const
{
	m_infoRenderTexture.resized(ScaledL(kInfoSize)).draw(Scaled(105) + LeftMargin(), Scaled(300));
}
