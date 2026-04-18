#pragma once
#include "UI/DialogBase.hpp"
#include "UI/LinearMenu.hpp"

/// @brief お気に入り追加ダイアログ(番号選択後、選択番号またはnoneを返す)
class FavoriteAddDialog : public DialogBase<Optional<int32>>
{
private:
	std::shared_ptr<noco::Canvas> m_canvas;
	LinearMenu m_menu;

	Co::Task<Optional<int32>> start() override;

	void draw() const override;

public:
	FavoriteAddDialog();
};

/// @brief 楽曲をお気に入りに追加
/// @param favoriteNumber お気に入り番号(1-9)
/// @param songPath 楽曲への相対パス
/// @return 追加に成功したらtrue(既に含まれている場合はfalse)
bool AddSongToFavorite(int32 favoriteNumber, StringView songPath);

/// @brief お気に入りファイルのパスを取得
/// @param favoriteNumber お気に入り番号(1-9)
/// @return .favファイルのフルパス
FilePath GetFavoriteFilePath(int32 favoriteNumber);
