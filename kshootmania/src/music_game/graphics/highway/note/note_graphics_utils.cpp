#include "note_graphics_utils.hpp"

namespace MusicGame::Graphics::NoteGraphicsUtils
{
	TiledTexture ApplyAlphaToNoteTexture(const Texture& texture, const TiledTextureSizeInfo& sizeInfo)
	{
		const PixelShader ps = HLSL{ U"shaders/multi_texture_mask.hlsl", U"PS" } | GLSL{ U"shaders/multi_texture_mask.frag",{ { U"PSConstants2D", 0 } } };
		if (!ps)
		{
			throw Error(U"Failed to load shader file 'shaders/multi_texture_mask.hlsl' or 'shaders/multi_texture_mask.frag'");
		}

		const Size textureSize = { sizeInfo.sourceSize.x * sizeInfo.column / 2, sizeInfo.sourceSize.y * sizeInfo.row };

		RenderTexture renderTextureRGB(textureSize);
		{
			const ScopedRenderTarget2D renderTarget(renderTextureRGB.clear(Palette::Black));
			for (int32 row = 0; row < sizeInfo.row; ++row)
			{
				for (int32 column = 0; column < sizeInfo.column / 2; ++column)
				{
					texture(sizeInfo.sourceSize.x * (column * 2), sizeInfo.sourceSize.y * row, sizeInfo.sourceSize)
						.draw(sizeInfo.sourceSize.x * column, sizeInfo.sourceSize.y * row);
				}
			}
		}

		RenderTexture renderTextureAlphaMask(textureSize);
		{
			const ScopedRenderTarget2D renderTarget(renderTextureAlphaMask.clear(Palette::Black));
			for (int32 row = 0; row < sizeInfo.row; ++row)
			{
				for (int32 column = 0; column < sizeInfo.column / 2; ++column)
				{
					texture(sizeInfo.sourceSize.x * (column * 2 + 1), sizeInfo.sourceSize.y * row, sizeInfo.sourceSize)
						.draw(sizeInfo.sourceSize.x * column, sizeInfo.sourceSize.y * row);
				}
			}
		}

		RenderTexture renderTextureMerged(textureSize);
		{
			Graphics2D::SetPSTexture(1, renderTextureAlphaMask);
			const ScopedCustomShader2D customShader(ps);
			const ScopedRenderTarget2D renderTarget(renderTextureMerged.clear(ColorF{ 0.0, 0.0 }));
			const ScopedRenderStates2D renderState(BlendState(
				true,
				Blend::SrcAlpha,
				Blend::InvSrcAlpha,
				BlendOp::Add,
				Blend::One));
			renderTextureRGB.draw();
		}

		TiledTextureSizeInfo sizeInfoHalfColumn = sizeInfo;
		sizeInfoHalfColumn.column /= 2;
		return TiledTexture(renderTextureMerged, sizeInfoHalfColumn);
	}

	int32 ChipNoteHeight(double yRate)
	{
		constexpr int32 kTableSize = 8;
		constexpr std::array<int32, kTableSize> kHeightTable = {
			14, 19, 23, 26, 28, 30, 32, 35
		};

		return kHeightTable[Clamp(static_cast<int32>(yRate * kTableSize), 0, kTableSize - 1)];
	}
}
