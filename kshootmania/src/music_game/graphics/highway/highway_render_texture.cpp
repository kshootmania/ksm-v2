#include "highway_render_texture.hpp"
#include "music_game/graphics/graphics_defines.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr Size RenderTextureSize(bool wide)
		{
			return wide ? kHighwayTextureSizeWide : kHighwayTextureSize;
		}

		constexpr Point RenderTextureOffsetPosition(bool wide)
		{
			return wide ? Point{ (kHighwayTextureSizeWide.x - kHighwayTextureSize.x) / 2, 0 } : Point::Zero();
		}

		constexpr StringView kBaseTextureFilename = U"base.gif";
		constexpr int32 kBaseTextureAdditiveColumn = 0;
		constexpr int32 kBaseTextureInvMultiplyColumn = 1;

		RenderTexture CreateBaseTexture(int32 column, const Size& size, const Color& clearColor)
		{
			const Texture textureAsset = TextureAsset(kBaseTextureFilename);

			RenderTexture texture(size);
			texture.clear(clearColor);

			// 中央へ描画
			{
				const ScopedRenderTarget2D renderTarget(texture);
				const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);
				textureAsset(kHighwayTextureSize.x * column, 0, kHighwayTextureSize).drawAt(size / 2);
			}

			return texture;
		}
	}

	HighwayRenderTexture::HighwayRenderTexture(bool wide)
		: m_additiveTexture(RenderTextureSize(wide))
		, m_invMultiplyTexture(RenderTextureSize(wide))
		, m_additiveBaseTexture(CreateBaseTexture(kBaseTextureAdditiveColumn, RenderTextureSize(wide), Palette::Black))
		, m_invMultiplyBaseTexture(CreateBaseTexture(kBaseTextureInvMultiplyColumn, RenderTextureSize(wide), Palette::Black))
		, m_offsetPosition(RenderTextureOffsetPosition(wide))
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

	const Point& HighwayRenderTexture::offsetPosition() const
	{
		return m_offsetPosition;
	}

	void HighwayRenderTexture::clearByBaseTexture() const
	{
		Shader::Copy(m_additiveBaseTexture, m_additiveTexture);
		Shader::Copy(m_invMultiplyBaseTexture, m_invMultiplyTexture);
	}
}
