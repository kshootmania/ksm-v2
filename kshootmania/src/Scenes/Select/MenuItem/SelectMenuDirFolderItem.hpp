#pragma once
#include "ISelectMenuItem.hpp"

class SelectMenuDirFolderItem : public ISelectMenuItem
{
private:
	const IsCurrentFolderYN m_isCurrentFolder;
	const FilePath m_fullPath;
	const String m_displayName;

public:
	explicit SelectMenuDirFolderItem(IsCurrentFolderYN isCurrentFolder, FilePathView fullPath);

	virtual ~SelectMenuDirFolderItem() = default;

	virtual void decide(const SelectMenuEventContext& context, int32 difficultyIdx) override;

	virtual bool isFolder() const override
	{
		return true;
	}

	virtual FilePathView fullPath() const override
	{
		return m_fullPath;
	}

	/// @brief Canvasのパラメータを設定(中央の項目)
	/// @param context イベントコンテキスト
	/// @param canvas 設定対象のCanvas
	/// @param difficultyIdx 現在選択中の難易度のインデックス(0～3)
	virtual void setCanvasParamsCenter(const SelectMenuEventContext& context, noco::Canvas& canvas, int32 difficultyIdx) const override;

	/// @brief Canvasのパラメータを設定(上下の項目)
	/// @param context イベントコンテキスト
	/// @param canvas 設定対象のCanvas
	/// @param difficultyIdx 現在選択中の難易度のインデックス(0～3)
	/// @param tag SubCanvasのタグ(例:"top1", "bottom2")
	virtual void setCanvasParamsTopBottom(const SelectMenuEventContext& context, noco::Canvas& canvas, int32 difficultyIdx, StringView tag) const override;

	virtual void requestGlyphPreload(int32 difficultyIdx) const override;

	/// @brief この項目をエクスプローラで表示
	/// @param difficultyIdx 現在選択中の難易度のインデックス(0～3)
	virtual void showInFileManager(int32 difficultyIdx) const override;
};
