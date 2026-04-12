#pragma once

namespace ImageUtils
{
	/// @brief Imageを指定サイズのタイルへ均等に分割
	/// @param image 分割対象の画像
	/// @param tileSize 1タイルのサイズ
	/// @return タイルのGrid(tiles[row][col]で行row列colにアクセス)
	[[nodiscard]]
	Grid<Image> SplitIntoTiles(const Image& image, Size tileSize);

	/// @brief 黒色を透明に置換したImageを生成
	[[nodiscard]]
	Image ReplaceBlackWithTransparent(const Image& image);

	/// @brief 画像ファイルをタイル単位のImageに分割して読み込み
	/// @param filePath 画像ファイルパス
	/// @param tileSize 1タイルのサイズ
	/// @return タイル群(tiles[row][col]で行row列colにアクセス、読み込み失敗時は空)
	/// @remarks Imageの最大サイズを超える大きいlayer画像の読み込み用
	[[nodiscard]]
	Grid<Image> LoadAsTileGrid(FilePathView filePath, Size tileSize);
}
