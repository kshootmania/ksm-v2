#pragma once
#include "UI/DialogBase.hpp"
#include "UI/LinearMenu.hpp"

enum class FavoriteRemoveChoice
{
	No = 0,
	Yes = 1,
};

/// @brief お気に入り削除確認ダイアログ(Yes/Noの選択結果を返す、キャンセル時はnone)
class FavoriteRemoveDialog : public DialogBase<Optional<FavoriteRemoveChoice>>
{
private:
	std::shared_ptr<noco::Canvas> m_canvas;
	LinearMenu m_menu;

	Co::Task<Optional<FavoriteRemoveChoice>> start() override;

	void draw() const override;

public:
	FavoriteRemoveDialog();
};
