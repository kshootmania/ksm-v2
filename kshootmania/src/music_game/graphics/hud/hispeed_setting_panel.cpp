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

		void DrawTextureRegionAtHalfRect(const TextureRegion& textureRegion, const RectF& halfRect)
		{
			textureRegion.resized(halfRect.w * 2, halfRect.h).draw(halfRect.pos);
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
		, m_numberLayoutHalf({ static_cast<double>(ScreenUtils::Scaled(10)) / 2, ScreenUtils::Scaled(9) }, TextureFontTextLayout::Align::Center)
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
				// TODO: もうちょっとどうにかする
				constexpr int32 kNumLetters = 4; // "x○.○"なので常に4文字(x9.9が上限なので桁数は考慮不要)
				assert(0 < hispeedSetting.value && "x-mod hispeed is out of range");
				assert(hispeedSetting.value < 100 && "x-mod hispeed is out of range");
				const auto grid = m_numberLayoutHalf.grid(anchorPosition, kNumLetters * 2 - 1); // 小数点"."の1文字のみ半分の幅にするので、2倍して1引く
				int32 doubledIndex = 1; // 幅が半分で末尾に余分に1個あるので2番目から始める
				m_targetValueNumberTextureFont.drawHalfGrid(grid, doubledIndex, hispeedSetting.value % 10); doubledIndex += 2; // 小数第1位
				DrawTextureRegionAtHalfRect(m_hispeedLetterTexture(kLetterTextureRowDot), grid.fromBack(doubledIndex++)); // "."
				m_targetValueNumberTextureFont.drawHalfGrid(grid, doubledIndex, hispeedSetting.value / 10); doubledIndex += 2; // 整数部分
				DrawTextureRegionAtHalfRect(m_hispeedLetterTexture(kLetterTextureRowX), grid.fromBack(doubledIndex)); // "x"
				break;
			}

			case HispeedType::OMod:
				m_targetValueNumberTextureFont.draw(m_numberLayout, anchorPosition, hispeedSetting.value, ZeroPaddingYN::No);
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
