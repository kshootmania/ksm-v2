#pragma once
#include "graphics/number_font_texture.hpp"
#include "ksh/chart_data.hpp"

namespace MusicGame::Graphics
{
	class SongInfoPanel
	{
	private:
		const Texture m_jacketTexture;
		const SizeF m_scaledJacketSize;

		const RenderTexture m_titlePanelBaseTexture;
		const RenderTexture m_detailPanelBaseTexture;
		const NumberFontTexture m_numberFontTexture;

	public:
		explicit SongInfoPanel(const ksh::ChartData& chartData);

		void draw() const;
	};
}
