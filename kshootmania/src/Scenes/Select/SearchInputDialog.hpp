#pragma once
#include "UI/DialogBase.hpp"

/// @brief 検索入力フェーズのダイアログ(Enterで確定したクエリ、キャンセルでnoneを返す)
class SearchInputDialog : public DialogBase<Optional<String>>
{
private:
	const String m_initialQuery;
	const std::function<void(StringView)> m_fnIncrementalUpdate;
	std::shared_ptr<noco::Canvas> m_canvas;

	Co::Task<Optional<String>> start() override;

	void draw() const override;

public:
	/// @brief コンストラクタ
	/// @param initialQuery TextBoxの初期テキスト(再入力時の復元用)
	/// @param fnIncrementalUpdate テキスト変化時のインクリメンタルサーチ用のコールバック
	SearchInputDialog(StringView initialQuery, std::function<void(StringView)> fnIncrementalUpdate);
};
