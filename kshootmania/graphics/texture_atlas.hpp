#pragma once

class TextureAtlas
{
private:
	const Texture m_texture;
	const int m_rowSize;
	const int m_columnSize;
	const Array<Rect> m_rects;

public:
	explicit TextureAtlas(StringView textureAssetKey, int rowSize = 1, int columnSize = 1);

	const Texture& texture() const
	{
		return m_texture;
	}

	TextureRegion operator()(int row, int column = 0) const;
};
