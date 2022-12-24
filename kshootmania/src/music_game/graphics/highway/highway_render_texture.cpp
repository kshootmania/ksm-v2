#include "highway_render_texture.hpp"
#include "music_game/graphics/graphics_defines.hpp"

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

	void HighwayRenderTexture::clearByBaseTexture() const
	{
		Shader::Copy(m_additiveBaseTexture, m_additiveTexture);
		Shader::Copy(m_invMultiplyBaseTexture, m_invMultiplyTexture);
	}
}
