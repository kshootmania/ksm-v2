#pragma once
#include <CoTaskLib.hpp>
#include "SelectMenuCacheDb.hpp"

/// @brief フォルダの読み込みを複数フレームに分けて進め、読み込み中はローディング表示と進捗を表示
/// @remark 読み込み完了時はtrue、キャンセル時はfalseを返す
class SelectFolderLoading : public Co::SequenceBase<bool>
{
private:
	SelectMenuCacheDb::Loader m_loader;
	std::shared_ptr<noco::Canvas> m_canvas;

	Co::Task<bool> start() override;

	void draw() const override;

	void updateCanvas();

public:
	explicit SelectFolderLoading(SelectMenuCacheDb::Loader&& loader);
};
