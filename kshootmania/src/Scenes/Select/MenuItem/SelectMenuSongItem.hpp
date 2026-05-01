#pragma once
#include "ISelectMenuItem.hpp"

using IsSingleChartItemYN = YesNo<struct IsSingleChartItemYN_tag>;

class SelectMenuSongItem : public ISelectMenuItem
{
private:
	const FilePath m_fullPath;

	bool m_chartExists = false;

	bool m_isSingleChartItem = false;

	std::array<std::shared_ptr<const SelectChartInfo>, kNumDifficulties> m_chartInfos;

	// 単一譜面項目の譜面情報を取得(存在しない場合はnullptrを返す)
	const SelectChartInfo* chartInfoForSingleChartItem() const;

	// 各難易度の存在有無とレベルの表示パラメータを設定
	void setDifficultyLevelDisplayParams(noco::Canvas& canvas) const;

public:
	// fullPathはディレクトリパスの場合は楽曲フォルダ、ファイルパスの場合は単体難易度の譜面として読み込む
	explicit SelectMenuSongItem(FilePathView fullPath);

	SelectMenuSongItem(FilePathView fullPath, const std::array<std::shared_ptr<const SelectChartInfo>, kNumDifficulties>& chartInfos, IsSingleChartItemYN isSingleChartItem);

	virtual ~SelectMenuSongItem() = default;

	virtual void decide(const SelectMenuEventContext& context, int32 difficultyIdx) override;

	virtual void decideAutoPlay(const SelectMenuEventContext& context, int32 difficultyIdx) override;

	virtual FilePathView fullPath() const override
	{
		return m_fullPath;
	}

	virtual const SelectChartInfo* chartInfoPtr(int difficultyIdx, FallbackForSingleChartYN fallbackForSingleChart = FallbackForSingleChartYN::Yes) const override;

	virtual bool difficultyMenuExists() const override
	{
		return !m_isSingleChartItem;
	}

	bool isSingleChartItem() const
	{
		return m_isSingleChartItem;
	}

	bool chartExists() const
	{
		return m_chartExists;
	}

	/// @brief お気に入り登録可能な項目かどうか
	/// @return お気に入り登録可能な場合はtrue
	virtual bool isFavoriteRegisterableItemType() const override
	{
		return true;
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

	/// @brief この項目をエクスプローラで表示
	/// @param difficultyIdx 現在選択中の難易度のインデックス(0～3)
	virtual void showInFileManager(int32 difficultyIdx) const override;

	/// @brief クリップボードにコピーする相対パスを取得
	/// @param difficultyIdx 現在選択中の難易度のインデックス(0～3)
	/// @return コピー対象の場合はsongsフォルダからの相対パス、コピー対象でない場合はnone
	virtual Optional<String> relativePathToCopy(int32 difficultyIdx) const override;

	/// @brief ハイスコア情報を取得
	/// @param context イベントコンテキスト
	/// @param difficultyIdx 難易度のインデックス(0～3)
	/// @return ハイスコア情報(存在しない場合はnone)
	virtual Optional<HighScoreInfo> highScoreInfo(const SelectMenuEventContext& context, int32 difficultyIdx) const override;

	virtual bool handleDifficultyChange(
		const SelectMenuEventContext& context,
		int32 currentDifficultyIdx,
		int32 delta) const override;

};
