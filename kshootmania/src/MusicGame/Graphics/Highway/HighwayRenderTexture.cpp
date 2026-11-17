#include "HighwayRenderTexture.hpp"
#include "MusicGame/Graphics/GraphicsDefines.hpp"
#include "MusicGame/Camera/CameraMath.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kBaseTextureFilename = U"base.gif";
		constexpr StringView kBaseWipeTextureFilename = U"base_wipe.gif";

		RenderTexture CreateBaseTexture(StringView filename, int32 column, const Color& clearColor)
		{
			const Texture textureAsset = TextureAsset(filename);

			RenderTexture texture(kHighwayTextureSizeWide);
			texture.clear(clearColor);

			// 中央へ描画
			{
				const ScopedRenderTarget2D renderTarget(texture);
				const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);
				textureAsset(kHighwayTextureSize.x * column, 0, kHighwayTextureSize).draw(kHighwayTextureOffsetX, 0);
			}

			return texture;
		}

		// 指定した高さの範囲を左右別々に描画
		void DrawBaseTextureRegion(const RenderTexture& baseTexture, int32 y, int32 height, double centerSplitShiftX)
		{
			if (height <= 0)
			{
				return;
			}

			constexpr double halfWidth = kHighwayTextureSize.x / 2;
			baseTexture(kHighwayTextureOffsetX, y, halfWidth, height).draw(kHighwayTextureOffsetX - centerSplitShiftX, y); // 左側
			baseTexture(kHighwayTextureOffsetX + halfWidth, y, halfWidth, height).draw(kHighwayTextureOffsetX + halfWidth + centerSplitShiftX, y); // 右側
		}
	}

	HighwayWipeRegion CalcHighwayWipeRegion(double highwayWipe)
	{
		// highway_wipeが正の場合は手前、負の場合は奥がレーン消失の範囲
		const int32 wipeHeight = static_cast<int32>(Math::Round(kHighwayTextureSize.y * Camera::HighwayWipeRate(highwayWipe)));
		const int32 baseHeight = kHighwayTextureSize.y - wipeHeight;
		return HighwayWipeRegion{
			.baseY = (highwayWipe < 0.0) ? wipeHeight : 0,
			.baseHeight = baseHeight,
			.wipeY = (highwayWipe < 0.0) ? 0 : baseHeight,
			.wipeHeight = wipeHeight,
		};
	}

	void DrawInHighwayWipeRegion(const TextureRegion& region, const Vec2& position, const HighwayWipeRegion& wipeRegion, const ColorF& color)
	{
		const double top = position.y;
		const double bottom = position.y + region.size.y;
		const double clippedTop = Max<double>(top, wipeRegion.wipeY);
		const double clippedBottom = Min<double>(bottom, wipeRegion.wipeY + wipeRegion.wipeHeight);
		if (clippedTop >= clippedBottom)
		{
			return;
		}

		const double uvHeight = region.uvRect.bottom - region.uvRect.top;
		TextureRegion clippedRegion = region;
		clippedRegion.uvRect.top = static_cast<float>(region.uvRect.top + uvHeight * (clippedTop - top) / region.size.y);
		clippedRegion.uvRect.bottom = static_cast<float>(region.uvRect.top + uvHeight * (clippedBottom - top) / region.size.y);
		clippedRegion.size.y = static_cast<float>(clippedBottom - clippedTop);
		clippedRegion.draw(Vec2{ position.x, clippedTop }, color);
	}

	HighwayRenderTexture::HighwayRenderTexture()
		: m_additiveTexture(kHighwayTextureSizeWide)
		, m_invMultiplyTexture(kHighwayTextureSizeWide)
		, m_additiveBaseTexture(CreateBaseTexture(kBaseTextureFilename, kTextureColumnMain, Palette::Black))
		, m_invMultiplyBaseTexture(CreateBaseTexture(kBaseTextureFilename, kTextureColumnSub, Palette::Black))
		, m_additiveBaseWipeTexture(CreateBaseTexture(kBaseWipeTextureFilename, kTextureColumnMain, Palette::Black))
		, m_invMultiplyBaseWipeTexture(CreateBaseTexture(kBaseWipeTextureFilename, kTextureColumnSub, Palette::Black))
	{
	}

	void HighwayRenderTexture::draw3D(const DynamicMesh& mesh) const
	{
		{
			const ScopedRenderStates3D renderState(kInvMultiply);
			mesh.draw(m_invMultiplyTexture);
		}

		{
			const ScopedRenderStates3D renderState(BlendState::Additive);
			mesh.draw(m_additiveTexture);
		}
	}

	const RenderTexture& HighwayRenderTexture::additiveTexture() const
	{
		return m_additiveTexture;
	}

	const RenderTexture& HighwayRenderTexture::invMultiplyTexture() const
	{
		return m_invMultiplyTexture;
	}

	void HighwayRenderTexture::drawBaseTexture(double centerSplit, double highwayWipe) const
	{
		const HighwayWipeRegion region = CalcHighwayWipeRegion(highwayWipe);

		if (MathUtils::AlmostEquals(centerSplit, 0.0) && region.wipeHeight == 0) [[likely]]
		{
			// center_splitとhighway_wipeが0の場合は一発で描画
			Shader::Copy(m_additiveBaseTexture, m_additiveTexture);
			Shader::Copy(m_invMultiplyBaseTexture, m_invMultiplyTexture);
			return;
		}

		const double centerSplitShiftX = Camera::CenterSplitShiftX(centerSplit);

		const ScopedRenderStates2D samplerState{ SamplerState::ClampNearest };

		m_additiveTexture.clear(Palette::Black);
		{
			const ScopedRenderTarget2D renderTarget(m_additiveTexture);
			DrawBaseTextureRegion(m_additiveBaseTexture, region.baseY, region.baseHeight, centerSplitShiftX);
			DrawBaseTextureRegion(m_additiveBaseWipeTexture, region.wipeY, region.wipeHeight, centerSplitShiftX);
		}

		m_invMultiplyTexture.clear(Palette::Black);
		{
			const ScopedRenderTarget2D renderTarget(m_invMultiplyTexture);
			DrawBaseTextureRegion(m_invMultiplyBaseTexture, region.baseY, region.baseHeight, centerSplitShiftX);
			DrawBaseTextureRegion(m_invMultiplyBaseWipeTexture, region.wipeY, region.wipeHeight, centerSplitShiftX);
		}
	}
}
