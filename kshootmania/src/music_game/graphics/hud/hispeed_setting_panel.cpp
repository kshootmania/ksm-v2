#include "hispeed_setting_panel.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kNumberTextureFontFilename = U"num2.png";
		constexpr StringView kFPSTextureFilename = U"fps.png";

		enum LetterTextureRow : int32
		{
			kLetterTextureRowDot = 0,
			kLetterTextureRowX,
			kLetterTextureRowC,
		};

		void DrawTextureRegionAtRect(const TextureRegion& textureRegion, const RectF& rect)
		{
			textureRegion.resized(rect.size).draw(rect.pos);
		}
	}

	HispeedSettingPanel::HispeedSettingPanel()
		: m_targetValueNumberTextureFont(kNumberTextureFontFilename, { 20, 18 }, { 0, 0 })
		, m_currentValueNumberTextureFont(kNumberTextureFontFilename, { 20, 18 }, { 20, 0 })
		, m_hispeedLetterTexture(kNumberTextureFontFilename,
			{
				.row = 3,
				.sourceOffset = { 0, 18 * 10 },
				.sourceSize = { 20, 18 },
			})
		, m_numberLayout(ScreenUtils::Scaled(10, 9), TextureFontTextLayout::Align::Center)
	{
	}

	void HispeedSettingPanel::draw(const Vec2& position, const Scroll::HighwayScrollContext& highwayScrollContext) const
	{
		using namespace ScreenUtils;
		const ScopedRenderStates2D samplerState(SamplerState::ClampLinear);

		const Scroll::HighwayScroll& highwayScroll = highwayScrollContext.highwayScroll();
		const HispeedSetting& hispeedSetting = highwayScroll.hispeedSetting();

		{
			const Vec2 anchorPosition = position - Scaled(29, 0);
			switch (hispeedSetting.type)
			{
			case HispeedType::XMod:
			{
				// TODO: "."の余白が大きい問題を修正
				constexpr int32 kNumLetters = 4; // "x○.○"なので常に4文字(x9.9が上限なので桁数は考慮不要)
				assert(0 < hispeedSetting.value && "x-mod hispeed is out of range");
				assert(hispeedSetting.value < 100 && "x-mod hispeed is out of range");
				const auto grid = m_numberLayout.grid(anchorPosition, kNumLetters);
				int32 index = 0;
				m_targetValueNumberTextureFont.draw(grid, index++, hispeedSetting.value % 10); // 小数第1位
				DrawTextureRegionAtRect(m_hispeedLetterTexture(kLetterTextureRowDot), grid.fromBack(index++)); // "."
				m_targetValueNumberTextureFont.draw(grid, index++, hispeedSetting.value / 10); // 整数部分
				DrawTextureRegionAtRect(m_hispeedLetterTexture(kLetterTextureRowX), grid.fromBack(index++)); // "x"
				break;
			}

			case HispeedType::OMod:
				m_currentValueNumberTextureFont.draw(m_numberLayout, anchorPosition, hispeedSetting.value, ZeroPaddingYN::No);
				break;

			case HispeedType::CMod:
			{
				const int32 numDigits = MathUtils::NumDigits(hispeedSetting.value);
				const int32 numLetters = numDigits + 1; // "C"の文字分があるので1文字多くなる
				const auto grid = m_numberLayout.grid(anchorPosition, numLetters);
				m_targetValueNumberTextureFont.draw(grid, 0, hispeedSetting.value);
				DrawTextureRegionAtRect(m_hispeedLetterTexture(kLetterTextureRowC), grid.fromBack(numDigits)); // "C"
				break;
			}

			default:
				assert(false && "Unknown hispeed type");
				break;
			}
		}

		m_currentValueNumberTextureFont.draw(m_numberLayout, position + Scaled(29, 0), highwayScroll.currentHispeed(), ZeroPaddingYN::No);
	}
}
