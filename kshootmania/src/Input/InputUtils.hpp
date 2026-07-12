#pragma once
#include <ksmaxis/ksmaxis.hpp>

namespace InputUtils
{
	ksmaxis::DeviceFlags GetLaserInputDeviceFlags();

	void InitKsmaxisForCurrentLaserInput();

	/// @brief ウィンドウのフォーカス状態を更新(メインループで毎フレーム呼ぶ。フレームごとに1回だけ更新されるため同一フレームで複数回呼んでも問題なし)
	void UpdateWindowFocusState();

	/// @brief ウィンドウがアクティブ化されたフレームかどうかを取得(ウィンドウをアクティブ化するためのクリックを入力として扱わないための判定に使用)
	[[nodiscard]]
	bool WindowFocusedThisFrame();

	/// @brief UIをマウスで操作可能かどうかを判定(マウス操作オプションが有効かつレーザー入力方式がマウスでない場合にtrue)
	[[nodiscard]]
	bool IsUIMouseInputEnabled();
}
