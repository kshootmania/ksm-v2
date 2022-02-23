#include "texture_atlas.hpp"

namespace
{
	Array<Rect> MakeRects(const Texture & texture, int rowSize, int columnSize)
	{
		const Size textureSize = texture.size();
		const int w = textureSize.x / columnSize;
		const int h = textureSize.y / rowSize;

		Array<Rect> rects;
		rects.reserve(columnSize * rowSize);
		for (int col = 0; col < columnSize; ++col)
		{
			for (int row = 0; row < rowSize; ++row)
			{
				rects.emplace_back(col * w, row * h, w, h);
			}
		}
		rects.shrink_to_fit();

		return rects;
	}
}

TextureAtlas::TextureAtlas(StringView textureAssetKey, int rowSize, int columnSize)
	: m_texture(TextureAsset(textureAssetKey))
	, m_rowSize(rowSize)
	, m_columnSize(columnSize)
	, m_rects(MakeRects(m_texture, rowSize, columnSize))
{
}

TextureRegion TextureAtlas::operator()(int row, int column) const
{
	if (row < 0 || m_rowSize <= row)
	{
		throw Error(U"TextureAtlas::draw(): row(={}) is out of range! (m_rowSize={})"_fmt(row, m_rowSize));
	}
	if (column < 0 || m_columnSize <= column)
	{
		throw Error(U"TextureAtlas::draw(): column(={}) is out of range! (m_columnSize={})"_fmt(column, m_columnSize));
	}

	int idx = column * m_rowSize + row;
	if (idx < 0 || std::ssize(m_rects) <= idx)
	{
		throw Error(U"TextureAtlas::draw(): idx(={}*{}+{}={}) is out of range! (m_rects.size()={})"_fmt(column, m_rowSize, row, idx, m_rects.size()));
	}

	return m_texture(m_rects[idx]);
}
