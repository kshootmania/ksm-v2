#pragma once
#include "ISelectMenuItem.hpp"
#include "Course/CourseInfo.hpp"

class SelectMenuCourseItem : public ISelectMenuItem
{
private:
	CourseInfo m_courseInfo;

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

	virtual void showInFileManager(int32 difficultyIdx) const override;

	/// @brief ハイスコア情報を取得
	/// @param context イベントコンテキスト
	/// @param difficultyIdx 難易度のインデックス(0～3、コースでは不使用)
	/// @return ハイスコア情報(存在しない場合はnone)
	virtual Optional<HighScoreInfo> highScoreInfo(const SelectMenuEventContext& context, int32 difficultyIdx) const override;
};
