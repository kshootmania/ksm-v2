#pragma once

class ICursorInputDevice
{
public:
	ICursorInputDevice() = default;

	virtual ~ICursorInputDevice() = default;

	/// @brief フレーム毎の更新(毎フレーム、deltaCursor使用前に呼ぶ)
	virtual void update() = 0;

	/// @brief 現在フレームでのカーソル移動数を返す
	/// @return カーソル移動数(移動していない場合は0)
	virtual int32 deltaCursor() const = 0;
};
