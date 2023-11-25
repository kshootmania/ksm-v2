#include "tiled_texture.hpp"

namespace
{
	TiledTextureSizeInfo AutoDetectSizeIfZero(TiledTextureSizeInfo sizeInfo, const Size& textureSize)
	{
		// kAutoDetectの判定はゼロ除算回避の判定を兼ねている
		constexpr int32 kAutoDetect = TiledTextureSizeInfo::kAutoDetect;
		static_assert(kAutoDetect == 0);

		if ((sizeInfo.row == kAutoDetect && sizeInfo.sourceSize.y == kAutoDetect) || (sizeInfo.column == kAutoDetect && sizeInfo.sourceSize.x == kAutoDetect))
		{
			Print << U"Warning[ AutoDetectSizeIfZero() ]: Could not auto-detect size because row & column & sourceSize are all zero!";
			sizeInfo.row = 1;
			sizeInfo.column = 1;
		}

		if (sizeInfo.row == kAutoDetect && sizeInfo.sourceSize.y != kAutoDetect)
		{
			sizeInfo.row = textureSize.y / sizeInfo.sourceSize.y;
		}

		if (sizeInfo.column == kAutoDetect && sizeInfo.sourceSize.x != kAutoDetect)
		{
			sizeInfo.column = textureSize.x / sizeInfo.sourceSize.x;
		}

		if (sizeInfo.sourceSize.x == kAutoDetect && sizeInfo.column != kAutoDetect)
		{
			sizeInfo.sourceSize.x = textureSize.x / sizeInfo.column;
		}

		if (sizeInfo.sourceSize.y == kAutoDetect && sizeInfo.row != kAutoDetect)
		{
			sizeInfo.sourceSize.y = textureSize.y / sizeInfo.row;
		}

		return sizeInfo;
	}
}

TiledTexture::TiledTexture(Texture&& texture, const TiledTextureSizeInfo& sizeInfo)
	: m_texture(std::move(texture))
	, m_sizeInfo(AutoDetectSizeIfZero(sizeInfo, m_texture.size()))
	, m_scaledSize(Scaled(m_sizeInfo.sourceScale, m_sizeInfo.sourceSize))
{
}

TiledTexture::TiledTexture(const Texture& texture, const TiledTextureSizeInfo& sizeInfo)
	: m_texture(texture)
	, m_sizeInfo(AutoDetectSizeIfZero(sizeInfo, m_texture.size()))
	, m_scaledSize(Scaled(m_sizeInfo.sourceScale, m_sizeInfo.sourceSize))
{
}

TiledTexture::TiledTexture(StringView textureAssetKey, const TiledTextureSizeInfo& sizeInfo)
	: m_texture(TextureAsset(textureAssetKey))
	, m_sizeInfo(AutoDetectSizeIfZero(sizeInfo, m_texture.size()))
	, m_scaledSize(Scaled(m_sizeInfo.sourceScale, m_sizeInfo.sourceSize))
#ifndef NDEBUG
	, m_textureAssetKey(textureAssetKey)
#endif
{
}

TiledTexture::TiledTexture(StringView textureAssetKey, SourceScale sourceScale)
	: TiledTexture(textureAssetKey, TiledTextureSizeInfo{ .sourceScale = sourceScale })
{
}

TextureRegion TiledTexture::operator()(int32 row, int32 column) const
{
	if (row < 0 || m_sizeInfo.row <= row)
	{
#ifdef NDEBUG
		Print << U"Warning[ TiledTexture::draw() ]: row(={}) is out of range! (min:0, max:{})"_fmt(row, m_sizeInfo.row - 1);
#else
		Print << U"Warning[ TiledTexture::draw(), \"{}\" ]: row(={}) is out of range! (min:0, max:{})"_fmt(m_textureAssetKey, row, m_sizeInfo.row - 1);
#endif
	}
	if (column < 0 || m_sizeInfo.column <= column)
	{
#ifdef NDEBUG
		Print << U"Warning[ TiledTexture::draw() ]: column(={}) is out of range! (min:0, max:{})"_fmt(column, m_sizeInfo.column - 1);
#else
		Print << U"Warning[ TiledTexture::draw(), \"{}\" ]: column(={}) is out of range! (min:0, max:{})"_fmt(m_textureAssetKey, column, m_sizeInfo.column - 1);
#endif
	}

	if (m_sizeInfo.sourceSize.x <= 0 || m_sizeInfo.sourceSize.y <= 0)
	{
#ifdef NDEBUG
		Print << U"Warning[ TiledTexture::draw() ]: sourceSize is invalid! ({})"_fmt(m_sizeInfo.sourceSize);
#else
		Print << U"Warning[ TiledTexture::draw(), \"{}\" ]: sourceSize is invalid! ({})"_fmt(m_textureAssetKey, m_sizeInfo.sourceSize);
#endif
		return TextureRegion{};
	}
	if (m_scaledSize.x <= 0 || m_scaledSize.y <= 0)
	{
#ifdef NDEBUG
		Print << U"Warning[ TiledTexture::draw() ]: m_scaledSize is invalid! ({})"_fmt(m_scaledSize);
#else
		Print << U"Warning[ TiledTexture::draw(), \"{}\" ]: m_scaledSize is invalid! ({})"_fmt(m_textureAssetKey, m_scaledSize);
#endif
		return TextureRegion{};
	}

	const double x = m_sizeInfo.sourceOffset.x + m_sizeInfo.sourceSize.x * column;
	const double y = m_sizeInfo.sourceOffset.y + m_sizeInfo.sourceSize.y * row;
	return m_texture(x, y, m_sizeInfo.sourceSize).resized(m_scaledSize);
}
