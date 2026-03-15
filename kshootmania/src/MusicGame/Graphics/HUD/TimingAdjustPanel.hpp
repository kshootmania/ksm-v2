#pragma once
#include "Graphics/NumberTextureFont.hpp"

namespace MusicGame::Graphics
{
	/// @brief TIMING ADJUST表示パネル(AutoSync/手動調整のオフセット値を表示)
	class TimingAdjustPanel
	{
	private:
		const Texture m_labelTexture;
		const Texture m_signTexture;
		const Texture m_msTexture;
		const NumberTextureFont m_numberFont;
		const TextureFontTextLayout m_numberLayout;

	public:
		TimingAdjustPanel();

		void draw(int32 timingAdjustMs) const;
	};
}
