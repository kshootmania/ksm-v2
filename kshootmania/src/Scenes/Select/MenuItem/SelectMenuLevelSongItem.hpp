#pragma once
#include "ISelectMenuItem.hpp"

/// @brief レベルソート用の兄弟難易度情報(メニューインデックスとレベル)
struct SiblingDifficultyInfo
{
	int32 menuIndex = -1;
	int32 level = -1;
};

/// @brief レベルソート用の楽曲項目(単一譜面+兄弟難易度リンク)
class SelectMenuLevelSongItem : public ISelectMenuItem
{
private:
	FilePath m_songDirectoryPath;
	int32 m_difficultyIdx;
	std::shared_ptr<const SelectChartInfo> m_chartInfo;
	std::array<SiblingDifficultyInfo, kNumDifficulties> m_siblings = {};

public:
	SelectMenuLevelSongItem(
		FilePathView chartFilePath,
		FilePathView songDirectoryPath,
		int32 difficultyIdx);

	SelectMenuLevelSongItem(
		std::shared_ptr<const SelectChartInfo> chartInfo,
		FilePathView songDirectoryPath,
		int32 difficultyIdx);

	virtual ~SelectMenuLevelSongItem() = default;

	/// @brief 兄弟難易度情報を設定(メニュー構築後のリンクパスで呼び出す)
	void setSiblings(const std::array<SiblingDifficultyInfo, kNumDifficulties>& siblings);

	[[nodiscard]]
	FilePathView songDirectoryPath() const;

	[[nodiscard]]
	int32 difficultyIdx() const;

	[[nodiscard]]
	bool chartExists() const;

	virtual void decide(const SelectMenuEventContext& context, int32 difficultyIdx) override;

	virtual void decideAutoPlay(const SelectMenuEventContext& context, int32 difficultyIdx) override;

	[[nodiscard]]
	virtual FilePathView fullPath() const override;

	[[nodiscard]]
	virtual const SelectChartInfo* chartInfoPtr(int difficultyIdx, FallbackForSingleChartYN fallbackForSingleChart = FallbackForSingleChartYN::Yes) const override;

	[[nodiscard]]
	virtual Optional<HighScoreInfo> highScoreInfo(const SelectMenuEventContext& context, int32 difficultyIdx) const override;

	[[nodiscard]]
	virtual bool difficultyMenuExists() const override;

	[[nodiscard]]
	virtual Optional<int32> itemDifficultyIdx() const override;

	virtual bool handleDifficultyChange(
		const SelectMenuEventContext& context,
		int32 currentDifficultyIdx,
		int32 delta) const override;

	[[nodiscard]]
	virtual bool isFavoriteRegisterableItemType() const override;

	virtual void setCanvasParamsCenter(const SelectMenuEventContext& context, noco::Canvas& canvas, int32 difficultyIdx) const override;

	virtual void setCanvasParamsTopBottom(const SelectMenuEventContext& context, noco::Canvas& canvas, int32 difficultyIdx, StringView tag) const override;

	virtual void showInFileManager(int32 difficultyIdx) const override;

	[[nodiscard]]
	virtual Optional<String> relativePathToCopy(int32 difficultyIdx) const override;
};
