#include "score_panel.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kCaptionTextureFilename = U"score_header.png";
		constexpr StringView kNumberTextureFontFilename = U"result_scorenum.png";
	}

	ScorePanel::ScorePanel()
		//: m_captionTexture(TextureAsset(kCaptionTextureFilename))
		: m_captionTexture(U"imgs/" + kCaptionTextureFilename)
		, m_numberTextureFont(kNumberTextureFontFilename, { 64, 64 })
		, m_numberLayout(ScreenUtils::Scaled(24, 24), TextureFontTextLayout::Align::Right, 8, ScreenUtils::Scaled(22))
	{
	}

	void ScorePanel::draw(int32 score) const
	{
		using namespace ScreenUtils;

		m_captionTexture.resized(Scaled(240, 24)).draw(Scene::Width() / 2 + Scaled(60), Scaled(16));
		m_numberTextureFont.draw(m_numberLayout, { Scene::Width() / 2 + Scaled(92), Scaled(42) }, score, ZeroPaddingYN::Yes);
	}
}
