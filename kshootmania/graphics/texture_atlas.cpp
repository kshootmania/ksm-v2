#include "texture_atlas.hpp"

namespace
{
	Array<Rect> MakeRects(const Texture & texture, int32 rowSize, int32 columnSize)
	{
		if (rowSize == 0)
		{
			return Array<Rect>();
		}

		const Size textureSize = texture.size();
		const int32 w = textureSize.x / columnSize;
		const int32 h = textureSize.y / rowSize;

		Array<Rect> rects;
		rects.reserve(columnSize * rowSize);
		for (int32 col = 0; col < columnSize; ++col)
		{
			for (int32 row = 0; row < rowSize; ++row)
			{
				rects.emplace_back(col * w, row * h, w, h);
			}
		}
		rects.shrink_to_fit();

		return rects;
	}
}

TextureAtlas::TextureAtlas(StringView textureAssetKey, int32 rowSize, int32 columnSize)
	: m_texture(TextureAsset(textureAssetKey))
	, m_rowSize(rowSize)
	, m_columnSize(columnSize)
	, m_rects(MakeRects(m_texture, rowSize, columnSize))
{
}

TextureRegion TextureAtlas::operator()(int32 row, int32 column) const
{
	if (row < 0 || m_rowSize <= row)
	{
		throw Error(U"TextureAtlas::draw(): row(={}) is out of range! (m_rowSize={})"_fmt(row, m_rowSize));
	}
	if (column < 0 || m_columnSize <= column)
	{
		throw Error(U"TextureAtlas::draw(): column(={}) is out of range! (m_columnSize={})"_fmt(column, m_columnSize));
	}

	int32 idx = column * m_rowSize + row;
	if (idx < 0 || std::ssize(m_rects) <= idx)
	{
		throw Error(U"TextureAtlas::draw(): idx(={}*{}+{}={}) is out of range! (m_rects.size()={})"_fmt(column, m_rowSize, row, idx, m_rects.size()));
	}

	return m_texture(m_rects[idx]);
}
