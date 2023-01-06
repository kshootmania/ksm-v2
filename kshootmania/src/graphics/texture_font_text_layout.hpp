#pragma once

/// @brief テクスチャ文字の描画座標レイアウト(NumberTextureFont等の描画座標計算に使用)
class TextureFontTextLayout
{
public:
	enum class Align
	{
		Center,
		Left,
		Right,
	};

	/// @brief レイアウトから取得されるグリッド
	class Grid
	{
	private:
		const Vec2 m_lastLetterPosition;
		const Vec2 m_diffPosition;
		const SizeF m_letterSize;
		const int32 m_numLettersAfterPadding;

	public:
		/// @brief コンストラクタ
		/// @param lastLetterPosition 最後の文字の左上座標
		/// @param diffPosition 1文字あたりの座標差分(文字方向とは逆方向で指定)
		/// @param letterSize 1文字の大きさ
		/// @param numLettersAfterPadding パディング後の文字数
		/// @note 最後の文字を起点に左向きに遡るような実装にしているのは、パディングが左方向に付く関係上、左端座標を計算するためには
		///       結局先に右端座標を計算する必要があり、右端を基準にしておいた方が余計な計算をしなくて済むため
		explicit Grid(const Vec2& lastLetterPosition, const Vec2& diffPosition, const SizeF& letterSize, int32 numLettersAfterPadding);

		/// @brief グリッドの始点(左端)から数えたインデックスで長方形を取得
		/// @param index 文字インデックス(負の値やm_numLettersAfterPadding以上の値でも問題ない)
		/// @return 長方形
		RectF fromFront(int32 index) const;

		/// @brief グリッドの末尾(右端)から数えたインデックスで長方形を取得
		/// @param index 文字インデックス(負の値やm_numLettersAfterPadding以上の値でも問題ない)
		/// @return 長方形
		RectF fromBack(int32 index) const;

		/// @brief パディング後の文字数を取得
		/// @return 文字数
		int32 numLettersAfterPadding() const;
	};

private:
	const SizeF m_letterSize;
	const Align m_align;
	const int32 m_numMaxPaddingLetters;
	const double m_scanX;

public:
	/// @brief コンストラクタ(余白なし)
	/// @param letterSize 1文字の大きさ
	/// @param align 文字揃え
	/// @param numMaxPaddingLetters パディング最大文字数
	explicit TextureFontTextLayout(const SizeF& letterSize, Align align, int32 numMaxPaddingLetters = 0);

	/// @brief コンストラクタ(scanX指定あり)
	/// @param letterSize 1文字の大きさ
	/// @param align 文字揃え
	/// @param numMaxPaddingLetters パディング最大文字数
	/// @param scanX 字送りの幅(余白なしであれば字幅と同じに設定すればよい)
	explicit TextureFontTextLayout(const SizeF& letterSize, Align align, int32 numMaxPaddingLetters, double scanX);

	/// @brief 配置先の座標と文字数をもとに描画先グリッドを取得
	/// @param position 配置先の座標
	/// @param numLetters 文字数
	/// @return グリッド
	Grid grid(const Vec2& position, int32 numLetters) const;
};
