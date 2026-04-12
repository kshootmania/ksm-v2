#include "ImageUtils.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#include <stb/stb_image.h>

namespace ImageUtils
{
	Grid<Image> SplitIntoTiles(const Image& image, Size tileSize)
	{
		if (tileSize.x <= 0 || tileSize.y <= 0)
		{
			return {};
		}

		const int32 cols = image.width() / tileSize.x;
		const int32 rows = image.height() / tileSize.y;
		if (cols <= 0 || rows <= 0)
		{
			return {};
		}

		Grid<Image> tiles(cols, rows);
		for (int32 row = 0; row < rows; ++row)
		{
			for (int32 col = 0; col < cols; ++col)
			{
				tiles[row][col] = image.clipped(col * tileSize.x, row * tileSize.y, tileSize.x, tileSize.y);
			}
		}
		return tiles;
	}

	Image ReplaceBlackWithTransparent(const Image& image)
	{
		Image result = image;
		for (auto& pixel : result)
		{
			if (pixel.r == 0 && pixel.g == 0 && pixel.b == 0)
			{
				pixel.a = 0;
			}
		}
		return result;
	}

	Grid<Image> LoadAsTileGrid(FilePathView filePath, Size tileSize)
	{
		if (tileSize.x <= 0 || tileSize.y <= 0)
		{
			return {};
		}

		// Imageの最大サイズを超える画像にも対応するため、stb_imageで直接デコードしてタイル単位に切り出す
		const Blob fileData{ filePath };
		if (fileData.isEmpty())
		{
			return {};
		}

		// stb_imageはバイト長をintで受け取るため上限を超えるサイズは扱えない
		if (fileData.size() > static_cast<size_t>(std::numeric_limits<int>::max()))
		{
			return {};
		}

		int imageW = 0;
		int imageH = 0;
		const std::unique_ptr<stbi_uc, void(*)(void*)> pixels(
			stbi_load_from_memory(
				reinterpret_cast<const stbi_uc*>(fileData.data()),
				static_cast<int>(fileData.size()),
				&imageW, &imageH, nullptr, STBI_rgb_alpha),
			&stbi_image_free);
		if (!pixels)
		{
			return {};
		}

		const int32 cols = imageW / tileSize.x;
		const int32 rows = imageH / tileSize.y;
		if (cols <= 0 || rows <= 0)
		{
			return {};
		}

		Grid<Image> tiles(cols, rows);
		const size_t tileRowBytes = static_cast<size_t>(tileSize.x) * 4;
		for (int32 row = 0; row < rows; ++row)
		{
			for (int32 col = 0; col < cols; ++col)
			{
				Image tile{ tileSize };
				for (int32 py = 0; py < tileSize.y; ++py)
				{
					const int32 srcY = row * tileSize.y + py;
					const stbi_uc* src = pixels.get() + (static_cast<size_t>(srcY) * imageW + col * tileSize.x) * 4;
					std::memcpy(tile[py], src, tileRowBytes);
				}
				tiles[row][col] = std::move(tile);
			}
		}
		return tiles;
	}
}
