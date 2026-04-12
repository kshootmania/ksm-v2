#pragma once
#include <Siv3D.hpp>
#include <NocoUI.hpp>
#include <CoTaskLib.hpp>

namespace NocoUtils
{
	/// @brief 指定したタグのTweenが再生完了するまで待機
	/// @param canvas キャンバス
	/// @param tag タグ名
	/// @return 待機タスク
	[[nodiscard]]
	Co::Task<void> WaitForTweenByTag(std::shared_ptr<noco::Canvas> canvas, const char32* tag);

	/// @brief ノード名のパスを辿ってノードを取得
	/// @param pCanvas キャンバスのポインタ
	/// @param path ノード名のパス
	/// @return ノード(見つからない場合はnullptr)
	[[nodiscard]]
	std::shared_ptr<noco::Node> GetNodeByPath(noco::Canvas* pCanvas, std::initializer_list<const char32_t*> path);

	/// @brief ノード名のパスを辿ってノードを取得(shared_ptrオーバーロード)
	/// @param pCanvas キャンバスのshared_ptr
	/// @param path ノード名のパス
	/// @return ノード(見つからない場合はnullptr)
	[[nodiscard]]
	std::shared_ptr<noco::Node> GetNodeByPath(const std::shared_ptr<noco::Canvas>& pCanvas, std::initializer_list<const char32_t*> path);

	/// @brief ノード名のパスを辿ってコンポーネントを取得
	/// @tparam TComponent 取得するコンポーネントの型
	/// @param pCanvas キャンバスのポインタ
	/// @param path ノード名のパス
	/// @return コンポーネント(見つからない場合はnullptr)
	template <class TComponent>
	[[nodiscard]]
	std::shared_ptr<TComponent> GetComponentByPath(noco::Canvas* pCanvas, std::initializer_list<const char32_t*> path)
	{
		if (const auto node = GetNodeByPath(pCanvas, path))
		{
			return node->getComponent<TComponent>();
		}
		return nullptr;
	}

	/// @brief ノード名のパスを辿ってコンポーネントを取得(shared_ptrオーバーロード)
	/// @tparam TComponent 取得するコンポーネントの型
	/// @param pCanvas キャンバスのshared_ptr
	/// @param path ノード名のパス
	/// @return コンポーネント(見つからない場合はnullptr)
	template <class TComponent>
	[[nodiscard]]
	std::shared_ptr<TComponent> GetComponentByPath(const std::shared_ptr<noco::Canvas>& pCanvas, std::initializer_list<const char32_t*> path)
	{
		return GetComponentByPath<TComponent>(pCanvas.get(), path);
	}

	/// @brief SubCanvas内のノードを名前で取得
	/// @param pCanvas キャンバスのポインタ
	/// @param tag SubCanvasのタグ
	/// @param nodeName ノード名
	/// @return ノード(見つからない場合はnullptr)
	[[nodiscard]]
	std::shared_ptr<noco::Node> GetSubCanvasNodeByName(noco::Canvas* pCanvas, StringView tag, StringView nodeName);

	/// @brief imgsフォルダ外のテクスチャキャッシュをクリア
	void UnloadExternalTextures();

	/// @brief SubCanvas内のコンポーネントを名前で取得
	/// @tparam TComponent 取得するコンポーネントの型
	/// @param pCanvas キャンバスのポインタ
	/// @param tag SubCanvasのタグ
	/// @param nodeName ノード名
	/// @return コンポーネント(見つからない場合はnullptr)
	template <class TComponent>
	[[nodiscard]]
	std::shared_ptr<TComponent> GetSubCanvasComponentByName(noco::Canvas* pCanvas, StringView tag, StringView nodeName)
	{
		if (const auto node = GetSubCanvasNodeByName(pCanvas, tag, nodeName))
		{
			return node->getComponent<TComponent>();
		}
		return nullptr;
	}
}
