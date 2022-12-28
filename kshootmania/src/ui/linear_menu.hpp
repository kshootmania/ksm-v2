#pragma once
#include <cassert>
#include "input/key_config.hpp"
#include "input/cursor/cursor_input.hpp"

using IsCyclicMenu = YesNo<struct IsCyclicMenu_tag>;

/// @brief 線形メニュー
class LinearMenu
{
private:
	CursorInput m_cursorInput;
	int32 m_cursorMin;
	int32 m_cursorMax;
	int32 m_cursor;
	int32 m_cursorStep;
	IsCyclicMenu m_cyclic;
	int32 m_deltaCursor = 0;

	void increment(int32 absDeltaCursor);

	void decrement(int32 absDeltaCursor);

public:
	/// @brief LinearMenuの生成パラメータ(enumの列挙子の個数を指定)
	struct CreateInfoWithEnumCount
	{
		CursorInput::CreateInfo cursorInputCreateInfo;
		int32 enumCount = 0;
		IsCyclicMenu cyclic = IsCyclicMenu::No;
		int32 defaultCursor = 0;
	};

	/// @brief LinearMenuの生成パラメータ(カーソルの最小値・最大値を指定)
	struct CreateInfoWithCursorMinMax
	{
		CursorInput::CreateInfo cursorInputCreateInfo;
		int32 cursorMin = 0;
		int32 cursorMax = 0;
		int32 cursorStep = 1;
		IsCyclicMenu cyclic = IsCyclicMenu::No;
		int32 defaultCursor = std::numeric_limits<int32>::min(); // cursorMinが負の値の場合にも未指定時にcursorMinの値へClampされるよう最小値を指定
	};

	explicit LinearMenu(const CreateInfoWithEnumCount& createInfo);

	explicit LinearMenu(const CreateInfoWithCursorMinMax& createInfo);

	/// @brief カーソルの値を取得
	/// @tparam T enumで取得したい場合は型を指定
	/// @return カーソルの値
	template <typename T = int32>
	T cursor() const;

	/// @brief カーソルの値を設定
	/// @tparam T enumをキャストなしで指定するためのテンプレートパラメータ(指定不要)
	/// @param value カーソルの値
	template <typename T>
	void setCursor(T value);

	/// @brief 更新(毎フレーム呼ぶ。入力を反映したくないときは呼ばないようにすればOK)
	void update();

	/// @brief カーソルの最小値を設定
	/// @tparam T enumをキャストなしで指定するためのテンプレートパラメータ(指定不要)
	/// @param value カーソルの最小値
	template <typename T>
	void setCursorMin(T value);

	/// @brief カーソルの最大値を設定
	/// @tparam T enumをキャストなしで指定するためのテンプレートパラメータ(指定不要)
	/// @param value カーソルの最大値
	template <typename T>
	void setCursorMax(T value);

	/// @brief カーソルの最大値を設定
	/// @tparam T enumをキャストなしで指定するためのテンプレートパラメータ(指定不要)
	/// @param valueMin カーソルの最小値
	/// @param valueMax カーソルの最大値
	template <typename T>
	void setCursorMinMax(T valueMin, T valueMax);

	/// @brief カーソル移動の1回あたりの変化量を設定
	/// @param step 1回あたりの変化量
	void setCursorStep(int32 step);

	/// @brief カーソルの値が最小値かどうか返す
	/// @return 最小値であればtrue、違えばfalse
	bool isCursorMin() const;

	/// @brief カーソルの値が最大値かどうか返す
	/// @return 最大値であればtrue、違えばfalse
	bool isCursorMax() const;

	/// @brief 前回のupdate時のカーソル値の変化量(update後にsetCursorによる変更があった場合は0になる)
	/// @return 変化量
	int32 deltaCursor() const;
};

template<typename T>
T LinearMenu::cursor() const
{
	return static_cast<T>(m_cursor);
}

template<typename T>
void LinearMenu::setCursor(T value)
{
	int32 cursor = static_cast<int32>(value);

	if (m_cyclic && m_cursorMin < m_cursorMax)
	{
		const int32 range = m_cursorMax + 1 - m_cursorMin;
		if (cursor < m_cursorMin)
		{
			while (cursor < m_cursorMin)
			{
				cursor += range;
			}
		}
		else
		{
			while (cursor > m_cursorMax)
			{
				cursor -= range;
			}
		}
	}

	m_cursor = Clamp(cursor, m_cursorMin, m_cursorMax);
	m_deltaCursor = 0;
}

template<typename T>
void LinearMenu::setCursorMin(T value)
{
	if (value > m_cursorMax)
	{
		assert(false && "LinearMenu::setCursorMin(): value must be smaller than or equal to m_cursorMax");
		value = m_cursorMax;
	}

	m_cursorMin = value;
	m_cursor = Clamp(m_cursor, m_cursorMin, m_cursorMax);
}

template<typename T>
void LinearMenu::setCursorMax(T value)
{
	if (value < m_cursorMin)
	{
		assert(false && "LinearMenu::setCursorMax(): value must be larger than or equal to m_cursorMin");
		value = m_cursorMin;
	}

	m_cursorMax = value;
	m_cursor = Clamp(m_cursor, m_cursorMin, m_cursorMax);
}

template<typename T>
void LinearMenu::setCursorMinMax(T valueMin, T valueMax)
{
	setCursorMin(valueMin);
	setCursorMax(valueMax);
}
