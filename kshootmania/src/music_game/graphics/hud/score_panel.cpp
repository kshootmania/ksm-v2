#include "score_panel.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kCaptionTextureFilename = U"score_header.png";
		constexpr StringView kNumberFontTextureFilename = U"result_scorenum.png";
	}

	ScorePanel::ScorePanel()
		//: m_captionTexture(TextureAsset(kCaptionTextureFilename))
		: m_captionTexture(U"imgs/" + kCaptionTextureFilename)
		, m_numberFontTexture(kNumberFontTextureFilename, ScreenUtils::Scaled(24, 24), { 64, 64 })
	{
	}

	void ScorePanel::draw(int32 score) const
	{
		using namespace ScreenUtils;

		m_captionTexture.resized(Scaled(240, 24)).draw(Scene::Width() / 2 + Scaled(60), Scaled(16));
		m_numberFontTexture.draw({ Scene::Width() / 2 + Scaled(92), Scaled(42) }, score, 8, Scaled(22), ZeroPaddingYN::Yes);
	}
}
