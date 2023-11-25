#include "option_menu.hpp"
#include "option_assets.hpp"

namespace
{
	constexpr Size kFieldTextureSourceSize = { 1200, 48 };
	constexpr Size kFieldTextureHalfSourceSize = { 600, 48 };

	constexpr int32 kFieldDiffY = 30;

	Array<OptionMenuField> MakeFields(const TiledTexture& fieldKeyTexture, const Array<OptionMenuField::CreateInfo>& fieldCreateInfos)
	{
		int32 i = 0;
		return fieldCreateInfos.map([&i, &fieldKeyTexture](const OptionMenuField::CreateInfo& createInfo) {
			const int32 keyTextureIdx = (createInfo.keyTextureIdx == OptionMenuField::CreateInfo::kKeyTextureIdxAutoSet) ? i : createInfo.keyTextureIdx;
			i++;
			return OptionMenuField(fieldKeyTexture(keyTextureIdx), createInfo);
		});
	}

	double MenuCursorAlphaValue(double sec)
	{
		constexpr double baseValue = 52.0 / 256;
		constexpr double maxValue = 188.0 / 256;
		constexpr double periodSec = Math::TwoPi * 0.15;
		constexpr double secOffset = 1.0 / 0.15;
		return Min(baseValue + (maxValue - baseValue) * Periodic::Sine0_1(periodSec, sec + secOffset), 1.0);
	}
}

OptionMenu::OptionMenu(StringView fieldKeyTextureAssetKey, const Array<OptionMenuField::CreateInfo>& fieldCreateInfos)
	: m_menu(
		LinearMenu::CreateInfoWithEnumCount{
			.cursorInputCreateInfo = {
				.type = CursorInput::Type::Vertical,
				.buttonFlags = CursorButtonFlags::kArrowOrBTOrLaser,
				.buttonIntervalSec = 0.1,
				.buttonIntervalSecFirst = 0.5,
			},
			.enumCount = static_cast<int32>(fieldCreateInfos.size()),
		})
	, m_fieldKeyTexture(fieldKeyTextureAssetKey,
		{
			.row = TiledTextureSizeInfo::kAutoDetect,
			.sourceScale = SourceScale::k2x,
			.sourceSize = kFieldTextureHalfSourceSize,
		})
	, m_fieldValueTexture(OptionTexture::kMenuFieldValue,
		{
			.row = 4,
			.sourceScale = SourceScale::k2x,
			.sourceSize = kFieldTextureHalfSourceSize,
		})
	, m_fieldCursorTexture(OptionTexture::kMenuFieldCursor,
		{
			.sourceScale = SourceScale::k2x,
			.sourceSize = kFieldTextureSourceSize,
		})
	, m_fields(MakeFields(m_fieldKeyTexture, fieldCreateInfos))
	, m_font(FontAsset(kFontAssetSystem))
	, m_stopwatch(StartImmediately::Yes)
{
}

void OptionMenu::update()
{
	m_menu.update();

	if (!m_fields.empty())
	{
		const int32 cursorIdx = m_menu.cursor();
		const int32 enumCount = static_cast<int32>(m_fields.size());
		if (cursorIdx < 0 || enumCount <= cursorIdx)
		{
			Print << U"Warning: Menu cursor is out of range! (func=OptionMenu::update(), cursor={}, min=0, max={})"_fmt(cursorIdx, enumCount - 1);
			return;
		}
		m_fields[cursorIdx].update();
	}
}

void OptionMenu::draw(const Vec2& position) const
{
	for (int32 i = 0; i < static_cast<int32>(m_fields.size()); ++i)
	{
		const Vec2 fieldPos = position + Vec2{ 0, Scaled(kFieldDiffY) * i };

		m_fields[i].draw(fieldPos, m_fieldValueTexture, m_font);

		// Draw cursor (additive)
		if (i == m_menu.cursor())
		{
			const ScopedRenderStates2D additive(BlendState::Additive);
			const double alpha = MenuCursorAlphaValue(m_stopwatch.sF());
			m_fieldCursorTexture().draw(fieldPos, ColorF{ 1.0, alpha });
		}
	}
}
