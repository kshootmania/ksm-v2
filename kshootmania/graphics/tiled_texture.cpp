#include "tiled_texture.hpp"

namespace
{
	TiledTextureSizeInfo AutoDetectSourceSizeIfZero(TiledTextureSizeInfo sizeInfo, const Size& textureSize)
	{
		if (sizeInfo.sourceSize.isZero())
		{
			sizeInfo.sourceSize.x = textureSize.x / sizeInfo.column;
			sizeInfo.sourceSize.y = textureSize.y / sizeInfo.row;
		}
		return sizeInfo;
	}
}

TiledTexture::TiledTexture(StringView textureAssetKey, const TiledTextureSizeInfo& sizeInfo)
	: m_texture(TextureAsset(textureAssetKey))
	, m_sizeInfo(AutoDetectSourceSizeIfZero(sizeInfo, m_texture.size()))
	, m_scaledSize(ScreenUtils::Scaled(m_sizeInfo.sourceScale, m_sizeInfo.sourceSize))
{
}

TiledTexture::TiledTexture(StringView textureAssetKey, ScreenUtils::SourceScale sourceScale)
	: TiledTexture(textureAssetKey, TiledTextureSizeInfo{ .sourceScale = sourceScale })
{
}

TextureRegion TiledTexture::operator()(int32 row, int32 column) const
{
	if (row < 0 || m_sizeInfo.row <= row)
	{
		Print << U"Warning[ TiledTexture::draw() ]: row(={}) is out of range! (min:0, max:{})"_fmt(row, m_sizeInfo.row - 1);
	}
	if (column < 0 || m_sizeInfo.column <= column)
	{
		Print << U"Warning[ TiledTexture::draw() ]: column(={}) is out of range! (min:0, max:{})"_fmt(column, m_sizeInfo.column - 1);
	}

	const double x = m_sizeInfo.offset.x + m_sizeInfo.sourceSize.x * column;
	const double y = m_sizeInfo.offset.y + m_sizeInfo.sourceSize.y * row;
	return m_texture(x, y, m_sizeInfo.sourceSize).resized(m_scaledSize);
}
