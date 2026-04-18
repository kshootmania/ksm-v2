#pragma once
#include "DialogBase.hpp"

/// @brief タイトルと本文を表示する汎用ダイアログ(simple_dialog.nocoを使用)
class SimpleDialog : public DialogBase<void>
{
private:
	std::shared_ptr<noco::Canvas> m_canvas;
	String m_titleText;
	String m_bodyText;

	Co::Task<void> start() override;

	void draw() const override;

public:
	SimpleDialog(String titleText, String bodyText);
};
