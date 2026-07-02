#include "NoteGraphicsUtils.hpp"
#include "Common/CommonDefines.hpp"

namespace MusicGame::Graphics::NoteGraphicsUtils
{
	TiledTexture ApplyAlphaToNoteTexture(const Texture& texture, const TiledTextureSizeInfo& sizeInfo)
	{
#ifdef __EMSCRIPTEN__
		const PixelShader ps = ESSL{ U"shaders/multi_texture_mask.essl.frag",{ { U"PSConstants2D", 0 } } };
#else
		const PixelShader ps = HLSL{ U"shaders/multi_texture_mask.hlsl", U"PS" } | GLSL{ U"shaders/multi_texture_mask.frag",{ { U"PSConstants2D", 0 } } };
#endif
		if (!ps)
		{
			throw Error(U"Failed to load multi_texture_mask shader");
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
		constexpr std::array<double, kTableSize> kHeightTable = {
			14.0, 19.0, 23.0, 26.0, 28.0, 30.0, 32.0, 35.0
		};

		const double dIdx = yRate * kTableSize;
		const int32 idx = static_cast<int32>(dIdx);
		const double lerpRate = dIdx - idx;
		return MathUtils::RoundToInt(std::lerp(kHeightTable[Clamp(idx, 0, kTableSize - 1)], kHeightTable[Clamp(idx + 1, 0, kTableSize - 1)], lerpRate));
	}

	int32 CalcChipNoteColorIndex(kson::Pulse y, const kson::BeatInfo& beatInfo, NoteSkinType noteSkin)
	{
		if (noteSkin == NoteSkinType::kDefault)
		{
			return 0;
		}

		const std::int64_t measureIdx = y / kson::kResolution;

		const auto& timeSigMap = beatInfo.timeSig;
		auto it = timeSigMap.upper_bound(measureIdx);
		if (it != timeSigMap.begin())
		{
			--it;
		}

		const kson::TimeSig& timeSig = it != timeSigMap.end() ? it->second : kson::TimeSig{ 4, 4 };
		const kson::Pulse measureStartY = measureIdx * kson::kResolution;
		const kson::Pulse positionInMeasure = y - measureStartY;
		const int32 div = kson::kResolution * 4 * timeSig.n / timeSig.d;

		if (div == 0)
		{
			return 0;
		}

		// 小節の分割数と色インデックス
		constexpr std::array<std::pair<int32, int32>, 8> kNoteTypeColorMap = {{
			{4, 1},
			{8, 2},
			{12, 3},
			{16, 4},
			{24, 5},
			{32, 6},
			{48, 7},
			{64, 8},
		}};

		if (positionInMeasure == 0)
		{
			return 1;
		}

		for (const auto& [noteType, colorIndex] : kNoteTypeColorMap)
		{
			if (div >= noteType && div % noteType == 0 && positionInMeasure % (div / noteType) == 0)
			{
				return colorIndex;
			}
		}

		return 0;
	}
}
