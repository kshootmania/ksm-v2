#pragma once
#include "ISelectMenuItem.hpp"
#include "Course/CourseInfo.hpp"

class SelectMenuCourseItem : public ISelectMenuItem
{
private:
	CourseInfo m_courseInfo;

	/// @brief コースの譜面リスト(ChartItemRoot)を最後に構築した際の記録
	struct ChartItemRootBuiltRecord
	{
		/// @brief 最後に構築したノード
		std::weak_ptr<noco::Node> node;

		/// @brief 最後に構築したコースのファイルパス
		FilePath courseFilePath;
	};

	/// @brief ChartItemRootを最後に構築した際の記録(ChartItemRootは全コース項目で共通のノードのため、記録も全項目で共通)
	/// @note 英字表記ONの場合は毎フレームsetCanvasParamsCenterが呼ばれるため、同じコースで連続して呼ばれた場合は再構築しない
	static ChartItemRootBuiltRecord s_chartItemRootBuiltRecord;

	/// @brief このコース項目のsetCanvasParamsCenterでChartItemRootを最後に処理したフレーム
	mutable int32 m_chartItemRootLastCalledFrame = -1;

public:
	explicit SelectMenuCourseItem(const CourseInfo& courseInfo);

	virtual ~SelectMenuCourseItem() = default;

	virtual void decide(const SelectMenuEventContext& context, int32 difficultyIdx) override;

	virtual void decideAutoPlay(const SelectMenuEventContext& context, int32 difficultyIdx) override;

	virtual FilePathView fullPath() const override
	{
		return m_courseInfo.filePath;
	}

	virtual bool isFolder() const override
	{
		return false;
	}

	virtual void setCanvasParamsCenter(const SelectMenuEventContext& context, noco::Canvas& canvas, int32 difficultyIdx) const override;

	virtual void setCanvasParamsTopBottom(const SelectMenuEventContext& context, noco::Canvas& canvas, int32 difficultyIdx, StringView tag) const override;

	virtual void requestGlyphPreload(int32 difficultyIdx) const override;

	virtual void showInFileManager(int32 difficultyIdx) const override;

	/// @brief ハイスコア情報を取得
	/// @param context イベントコンテキスト
	/// @param difficultyIdx 難易度のインデックス(0～3、コースでは不使用)
	/// @return ハイスコア情報(存在しない場合はnone)
	virtual Optional<HighScoreInfo> highScoreInfo(const SelectMenuEventContext& context, int32 difficultyIdx) const override;
};
