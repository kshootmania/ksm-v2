#pragma once
#include "LinearMenu.hpp"

template <typename T>
class ArrayWithLinearMenu
{
private:
	[[nodiscard]]
	static LinearMenu::CreateInfoWithCursorMinMax FixLinearMenuCreateInfo(const Array<T>& array, LinearMenu::CreateInfoWithCursorMinMax createInfo);

	Array<T> m_array;
	LinearMenu m_linearMenu;

	void updateLinearMenuCursorMax();

public:
	explicit ArrayWithLinearMenu(const LinearMenu::CreateInfoWithCursorMinMax& createInfo);

	explicit ArrayWithLinearMenu(const Array<T>& array, const LinearMenu::CreateInfoWithCursorMinMax& createInfo);

	explicit ArrayWithLinearMenu(Array<T>&& array, const LinearMenu::CreateInfoWithCursorMinMax& createInfo);

	[[nodiscard]]
	int32 cursor() const;

	template <typename U>
	[[nodiscard]]
	U cursorAs() const;

	template <typename U>
	void setCursor(U value);

	/// @brief 配列内で指定した値に一致する要素のインデックスにカーソルを設定
	/// @param value 検索する値
	/// @return 値が見つかった場合はtrue、見つからなかった場合はfalse(カーソルは変更されない)
	bool setCursorToValue(const T& value);

	void update();

	/// @brief 外部からのカーソル移動量を加えて更新(クリック操作によるカーソル移動用)
	/// @param externalDelta 外部からのカーソル移動量
	void updateWithExternalDelta(int32 externalDelta);

	[[nodiscard]]
	bool isCursorMin() const;

	[[nodiscard]]
	bool isCursorMax() const;

	[[nodiscard]]
	int32 deltaCursor() const;

	[[nodiscard]]
	double cursorRate() const;

	[[nodiscard]]
	T& cursorValue();

	[[nodiscard]]
	const T& cursorValue() const;

	void setArray(const Array<T>& array);

	[[nodiscard]]
	auto& operator[](std::size_t idx);

	[[nodiscard]]
	const auto& operator[](std::size_t idx) const;

	[[nodiscard]]
	auto& at(std::size_t idx);

	[[nodiscard]]
	const auto& at(std::size_t idx) const;

	[[nodiscard]]
	auto& atCyclic(int32 idx);

	[[nodiscard]]
	const auto& atCyclic(int32 idx) const;

	void clear();

	void push_back(T&& value);

	void push_back(const T& value);

	template <typename... Args>
	void emplace_back(Args&&... args);

	void pop_back();

	[[nodiscard]]
	auto begin();

	[[nodiscard]]
	auto begin() const;

	[[nodiscard]]
	auto cbegin() const;

	[[nodiscard]]
	auto end();

	[[nodiscard]]
	auto end() const;

	[[nodiscard]]
	auto cend() const;

	[[nodiscard]]
	auto rbegin();

	[[nodiscard]]
	auto rbegin() const;

	[[nodiscard]]
	auto crbegin() const;

	[[nodiscard]]
	auto rend();

	[[nodiscard]]
	auto rend() const;

	[[nodiscard]]
	auto crend() const;

	[[nodiscard]]
	auto& back();

	[[nodiscard]]
	const auto& back() const;

	[[nodiscard]]
	auto size() const;

	[[nodiscard]]
	auto empty() const;
};

template<typename T>
LinearMenu::CreateInfoWithCursorMinMax ArrayWithLinearMenu<T>::FixLinearMenuCreateInfo(const Array<T>& array, LinearMenu::CreateInfoWithCursorMinMax createInfo)
{
	// cursorMinとcursorMaxは配列のサイズをもとに決まる
	createInfo.cursorMin = 0;
	createInfo.cursorMax = Max(static_cast<int32>(array.size()) - 1, 0);
	return createInfo;
}

template <typename T>
int32 ArrayWithLinearMenu<T>::cursor() const
{
	return m_linearMenu.cursor();
}

template <typename T>
template <typename U>
U ArrayWithLinearMenu<T>::cursorAs() const
{
	return m_linearMenu.template cursorAs<U>();
}

template <typename T>
template <typename U>
void ArrayWithLinearMenu<T>::setCursor(U value)
{
	m_linearMenu.setCursor(value);
}

template <typename T>
bool ArrayWithLinearMenu<T>::setCursorToValue(const T& value)
{
	for (int32 i = 0; i < static_cast<int32>(m_array.size()); ++i)
	{
		if (m_array[i] == value)
		{
			m_linearMenu.setCursor(i);
			return true;
		}
	}
	return false;
}

template <typename T>
void ArrayWithLinearMenu<T>::updateLinearMenuCursorMax()
{
	m_linearMenu.setCursorMax(std::max(static_cast<int32>(m_array.size()) - 1, 0));
}

template<typename T>
ArrayWithLinearMenu<T>::ArrayWithLinearMenu(const LinearMenu::CreateInfoWithCursorMinMax& createInfo)
	: m_linearMenu(FixLinearMenuCreateInfo(m_array, createInfo)) // m_arrayの宣言はm_linearMenuより先なのでここで使用するのは問題なし
{
}

template<typename T>
ArrayWithLinearMenu<T>::ArrayWithLinearMenu(const Array<T>& array, const LinearMenu::CreateInfoWithCursorMinMax& createInfo)
	: m_array(array)
	, m_linearMenu(FixLinearMenuCreateInfo(m_array, createInfo))
{
}

template<typename T>
ArrayWithLinearMenu<T>::ArrayWithLinearMenu(Array<T>&& array, const LinearMenu::CreateInfoWithCursorMinMax& createInfo)
	: m_array(std::move(array))
	, m_linearMenu(FixLinearMenuCreateInfo(m_array, createInfo))
{
}

template <typename T>
void ArrayWithLinearMenu<T>::update()
{
	m_linearMenu.update();
}

template <typename T>
void ArrayWithLinearMenu<T>::updateWithExternalDelta(int32 externalDelta)
{
	m_linearMenu.updateWithExternalDelta(externalDelta);
}

template <typename T>
bool ArrayWithLinearMenu<T>::isCursorMin() const
{
	return m_linearMenu.isCursorMin();
}

template <typename T>
bool ArrayWithLinearMenu<T>::isCursorMax() const
{
	return m_linearMenu.isCursorMax();
}

template<typename T>
int32 ArrayWithLinearMenu<T>::deltaCursor() const
{
	return m_linearMenu.deltaCursor();
}

template<typename T>
double ArrayWithLinearMenu<T>::cursorRate() const
{
	return m_linearMenu.cursorRate();
}

template <typename T>
T& ArrayWithLinearMenu<T>::cursorValue()
{
	return m_array.at(m_linearMenu.cursor());
}

template <typename T>
const T& ArrayWithLinearMenu<T>::cursorValue() const
{
	return m_array.at(m_linearMenu.cursor());
}

template <typename T>
void ArrayWithLinearMenu<T>::setArray(const Array<T>& array)
{
	m_array = array;
}

template <typename T>
auto& ArrayWithLinearMenu<T>::operator[](std::size_t idx)
{
	return m_array[idx];
}

template <typename T>
const auto& ArrayWithLinearMenu<T>::operator[](std::size_t idx) const
{
	return m_array[idx];
}

template <typename T>
auto& ArrayWithLinearMenu<T>::at(std::size_t idx)
{
	return m_array.at(idx);
}

template <typename T>
const auto& ArrayWithLinearMenu<T>::at(std::size_t idx) const
{
	return m_array.at(idx);
}

template<typename T>
auto& ArrayWithLinearMenu<T>::atCyclic(int32 idx)
{
	const int32 size = static_cast<int32>(m_array.size());
	if (size == 0)
	{
		throw Error{ U"ArrayWithLinearMenu::atCyclic() called on empty array" };
	}

	if (idx < 0)
	{
		while (idx < 0)
		{
			idx += size;
		}
	}
	else
	{
		while (idx >= size)
		{
			idx -= size;
		}
	}

	return at(idx);
}

template<typename T>
const auto& ArrayWithLinearMenu<T>::atCyclic(int32 idx) const
{
	const int32 size = static_cast<int32>(m_array.size());
	if (size == 0)
	{
		throw Error{ U"ArrayWithLinearMenu::atCyclic() called on empty array" };
	}

	if (idx < 0)
	{
		while (idx < 0)
		{
			idx += size;
		}
	}
	else
	{
		while (idx >= size)
		{
			idx -= size;
		}
	}

	return at(idx);
}

template <typename T>
void ArrayWithLinearMenu<T>::clear()
{
	m_array.clear();
	updateLinearMenuCursorMax();
}

template <typename T>
void ArrayWithLinearMenu<T>::push_back(T&& value)
{
	m_array.push_back(std::move(value));
	updateLinearMenuCursorMax();
}

template <typename T>
void ArrayWithLinearMenu<T>::push_back(const T& value)
{
	m_array.push_back(value);
	updateLinearMenuCursorMax();
}

template <typename T>
template<typename ...Args>
void ArrayWithLinearMenu<T>::emplace_back(Args && ...args)
{
	m_array.emplace_back(std::forward<Args>(args)...);
	updateLinearMenuCursorMax();
}

template <typename T>
void ArrayWithLinearMenu<T>::pop_back()
{
	m_array.pop_back();
	updateLinearMenuCursorMax();
}

template <typename T>
auto ArrayWithLinearMenu<T>::begin()
{
	return m_array.begin();
}

template <typename T>
auto ArrayWithLinearMenu<T>::begin() const
{
	return m_array.begin();
}

template <typename T>
auto ArrayWithLinearMenu<T>::cbegin() const
{
	return m_array.cbegin();
}

template <typename T>
auto ArrayWithLinearMenu<T>::end()
{
	return m_array.end();
}

template <typename T>
auto ArrayWithLinearMenu<T>::end() const
{
	return m_array.end();
}

template <typename T>
auto ArrayWithLinearMenu<T>::cend() const
{
	return m_array.cend();
}

template <typename T>
auto ArrayWithLinearMenu<T>::rbegin()
{
	return m_array.rbegin();
}

template <typename T>
auto ArrayWithLinearMenu<T>::rbegin() const
{
	return m_array.rbegin();
}

template <typename T>
auto ArrayWithLinearMenu<T>::crbegin() const
{
	return m_array.crbegin();
}

template <typename T>
auto ArrayWithLinearMenu<T>::rend()
{
	return m_array.rend();
}

template <typename T>
auto ArrayWithLinearMenu<T>::rend() const
{
	return m_array.rend();
}

template <typename T>
auto ArrayWithLinearMenu<T>::crend() const
{
	return m_array.crend();
}

template <typename T>
auto& ArrayWithLinearMenu<T>::back()
{
	return m_array.back();
}

template <typename T>
const auto& ArrayWithLinearMenu<T>::back() const
{
	return m_array.back();
}

template <typename T>
auto ArrayWithLinearMenu<T>::size() const
{
	return m_array.size();
}

template <typename T>
auto ArrayWithLinearMenu<T>::empty() const
{
	return m_array.empty();
}
