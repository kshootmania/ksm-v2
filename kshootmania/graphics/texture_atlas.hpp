#pragma once

class TextureAtlas
{
private:
	const Texture m_texture;
	const int32 m_rowSize;
	const int32 m_columnSize;
	const Array<Rect> m_rects;

public:
	explicit TextureAtlas(StringView textureAssetKey, int32 rowSize = 1, int32 columnSize = 1);

	const Texture& texture() const
	{
		return m_texture;
	}

	TextureRegion operator()(int32 row, int32 column = 0) const;
};
