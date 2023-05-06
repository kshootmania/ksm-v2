#include "highway_render_texture.hpp"
#include "music_game/graphics/graphics_defines.hpp"
#include "music_game/camera/camera_math.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kBaseTextureFilename = U"base.gif";

		RenderTexture CreateBaseTexture(int32 column, const Color& clearColor)
		{
			const Texture textureAsset = TextureAsset(kBaseTextureFilename);

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
	}

	HighwayRenderTexture::HighwayRenderTexture()
		: m_additiveTexture(kHighwayTextureSizeWide)
		, m_invMultiplyTexture(kHighwayTextureSizeWide)
		, m_additiveBaseTexture(CreateBaseTexture(kTextureColumnMain, Palette::Black))
		, m_invMultiplyBaseTexture(CreateBaseTexture(kTextureColumnSub, Palette::Black))
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

	void HighwayRenderTexture::drawBaseTexture(double centerSplit) const
	{
		if (MathUtils::AlmostEquals(centerSplit, 0.0)) [[likely]]
		{
			Shader::Copy(m_additiveBaseTexture, m_additiveTexture);
			Shader::Copy(m_invMultiplyBaseTexture, m_invMultiplyTexture);
			return;
		}

		const double centerSplitShiftX = Camera::CenterSplitShiftX(centerSplit);
		const Size halfHighwayTextureSize = { kHighwayTextureSizeWide.x / 2, kHighwayTextureSizeWide.y };

		const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);

		m_additiveTexture.clear(Palette::Black);
		{
			const ScopedRenderTarget2D renderTarget(m_additiveTexture);
			m_additiveBaseTexture(0, 0, halfHighwayTextureSize).draw(-centerSplitShiftX, 0);
			m_additiveBaseTexture(halfHighwayTextureSize.x, 0, halfHighwayTextureSize).draw(halfHighwayTextureSize.x + centerSplitShiftX, 0);
		}

		m_invMultiplyTexture.clear(Palette::Black);
		{
			const ScopedRenderTarget2D renderTarget(m_invMultiplyTexture);
			m_invMultiplyBaseTexture(0, 0, halfHighwayTextureSize).draw(-centerSplitShiftX, 0);
			m_invMultiplyBaseTexture(halfHighwayTextureSize.x, 0, halfHighwayTextureSize).draw(halfHighwayTextureSize.x + centerSplitShiftX, 0);
		}
	}
}
