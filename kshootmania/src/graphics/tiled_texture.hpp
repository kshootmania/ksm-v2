#pragma once

struct TiledTextureSizeInfo
{
	static constexpr int32 kAutoDetect = 0;
	static constexpr Size kAutoDetectSize = { 0, 0 };

	// row・columnに対してkAutoDetectを指定するとsourceSizeに応じて自動的に決定される
	// (その場合、sourceSizeにはkAutoDetectSize以外を指定すること)
	int32 row = 1;
	int32 column = 1;

	Point sourceOffset = { 0, 0 };
	SourceScale sourceScale = SourceScale::kNoScaling;

	// sourceSizeに対してkAutoDetectSizeを指定するとrow・columnに応じて自動的に決定される
	// (その場合、row・columnにはkAutoDetect以外を指定すること)
	Size sourceSize = kAutoDetectSize;
};

class TiledTexture
{
private:
	const Texture m_texture;
	const TiledTextureSizeInfo m_sizeInfo;
	const Size m_scaledSize;

#ifndef NDEBUG
	const String m_textureAssetKey;
#endif

public:
	TiledTexture(Texture&& texture, const TiledTextureSizeInfo& sizeInfo);
	TiledTexture(const Texture& texture, const TiledTextureSizeInfo& sizeInfo);
	TiledTexture(StringView textureAssetKey, const TiledTextureSizeInfo& sizeInfo);
	TiledTexture(StringView textureAssetKey, SourceScale scale);

	const Size& scaledSize() const
	{
		return m_scaledSize;
	}

	TextureRegion operator()(int32 row = 0, int32 column = 0) const;

	int32 row() const
	{
		return m_sizeInfo.row;
	}

	int32 column() const
	{
		return m_sizeInfo.column;
	}
};
