#pragma once

struct TiledTextureSizeInfo
{
	int32 row = 1;
	int32 column = 1;
	Point offset = { 0, 0 };
	ScreenUtils::SourceScale sourceScale = ScreenUtils::SourceScale::kUnspecified;
	Size sourceSize = { 0, 0 };
};

class TiledTexture
{
private:
	const Texture m_texture;
	const TiledTextureSizeInfo m_sizeInfo;
	const Size m_scaledSize;

public:
	TiledTexture(StringView textureAssetKey, const TiledTextureSizeInfo& sizeInfo);
	TiledTexture(StringView textureAssetKey, ScreenUtils::SourceScale scale);

	const Texture& texture() const
	{
		return m_texture;
	}

	const Size& scaledSize() const
	{
		return m_scaledSize;
	}

	TextureRegion operator()(int32 row = 0, int32 column = 0) const;
};
