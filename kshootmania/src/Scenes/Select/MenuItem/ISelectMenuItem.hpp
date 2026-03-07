#pragma once
#include "Scenes/Select/SelectMenu.hpp"
#include "Scenes/Select/SelectChartInfo.hpp"
#include "HighScore/HighScoreInfo.hpp"

struct SelectMenuItemGraphicAssets;

using IsCurrentFolderYN = YesNo<struct IsCurrentFolderYN_tag>;
using FallbackForSingleChartYN = YesNo<struct FallbackForSingleChartYN_tag>;

class ISelectMenuItem
{
protected:
	static String FolderDisplayNameCenter(StringView folderName, IsCurrentFolderYN isCurrentFolder)
	{
		if (isCurrentFolder)
		{
			return U"<<   {}   <<"_fmt(folderName);
		}
		else
		{
			return U">>   {}   >>"_fmt(folderName);
		}
	}

	static String FolderDisplayNameTopBottom(StringView folderName, IsCurrentFolderYN isCurrentFolder)
	{
		if (isCurrentFolder)
		{
			return U"<<   {}     "_fmt(folderName);
		}
		else
		{
			return U"     {}   >>"_fmt(folderName);
		}
	}

public:
	ISelectMenuItem() = default;

	virtual ~ISelectMenuItem() = default;

	virtual void decide(const SelectMenuEventContext& context, int32 difficultyIdx) = 0;

	virtual void decideAutoPlay([[maybe_unused]] const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
	{
	}

	virtual FilePathView fullPath() const = 0;

	/// @brief 譜面情報を取得
	/// @param difficultyIdx 難易度のインデックス(0～3)
	/// @param fallbackForSingleChart 単一譜面アイテムの場合、difficultyIdxに関係なく譜面情報を返すかどうか
	/// @return 譜面情報のポインタ(存在しない場合はnullptr)
	virtual const SelectChartInfo* chartInfoPtr([[maybe_unused]] int difficultyIdx, [[maybe_unused]] FallbackForSingleChartYN fallbackForSingleChart = FallbackForSingleChartYN::Yes) const
	{
		return nullptr;
	}

	/// @brief ハイスコア情報を取得
	/// @param difficultyIdx 難易度のインデックス(0～3)
	/// @return ハイスコア情報(存在しない場合はnone)
	virtual Optional<HighScoreInfo> highScoreInfo([[maybe_unused]] int32 difficultyIdx) const
	{
		return none;
	}

	virtual bool difficultyMenuExists() const
	{
		return false;
	}

	/// @brief この項目自体が表す難易度インデックス(レベルソート項目等で使用)
	virtual Optional<int32> itemDifficultyIdx() const
	{
		return none;
	}

	/// @brief 難易度変更入力を処理
	/// @param context イベントコンテキスト
	/// @param currentDifficultyIdx 現在の難易度カーソル値(0～3)
	/// @param delta カーソルの変化量(正:右, 負:左)
	/// @return 難易度変更が行われた場合はtrue
	virtual bool handleDifficultyChange(
		[[maybe_unused]] const SelectMenuEventContext& context,
		[[maybe_unused]] int32 currentDifficultyIdx,
		[[maybe_unused]] int32 delta) const
	{
		return false;
	}

	virtual bool isSubFolderHeading() const
	{
		return false;
	}

	virtual bool isFolder() const
	{
		return false;
	}

	/// @brief お気に入り登録可能な項目かどうか
	/// @return お気に入り登録可能な場合はtrue
	virtual bool isFavoriteRegisterableItemType() const
	{
		return false;
	}

	/// @brief Canvasのパラメータを設定(中央の項目)
	/// @param context イベントコンテキスト
	/// @param canvas 設定対象のCanvas
	/// @param difficultyIdx 現在選択中の難易度のインデックス(0～3)
	virtual void setCanvasParamsCenter(const SelectMenuEventContext& context, noco::Canvas& canvas, int32 difficultyIdx) const = 0;

	/// @brief Canvasのパラメータを設定(上下の項目)
	/// @param context イベントコンテキスト
	/// @param canvas 設定対象のCanvas
	/// @param difficultyIdx 現在選択中の難易度のインデックス(0～3)
	/// @param tag SubCanvasのタグ(例:"top1", "bottom2")
	virtual void setCanvasParamsTopBottom(const SelectMenuEventContext& context, noco::Canvas& canvas, int32 difficultyIdx, StringView tag) const = 0;

	/// @brief この項目をエクスプローラで表示
	/// @param difficultyIdx 現在選択中の難易度のインデックス(0～3)
	virtual void showInFileManager([[maybe_unused]] int32 difficultyIdx) const
	{
	}

	/// @brief クリップボードにコピーする相対パスを取得
	/// @param difficultyIdx 現在選択中の難易度のインデックス(0～3)
	/// @return コピー対象の場合はsongsフォルダからの相対パス、コピー対象でない場合はnone
	virtual Optional<String> relativePathToCopy([[maybe_unused]] int32 difficultyIdx) const
	{
		return none;
	}
};
