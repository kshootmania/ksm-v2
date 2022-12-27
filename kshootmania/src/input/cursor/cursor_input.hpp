#pragma once
#include <memory>
#include "button_cursor_input_device.hpp"

namespace CursorButtonFlags
{
	enum CursorButtonFlags : int32
	{
		kNone = 0,
		kArrow = 1 << 0,
		kBT = 1 << 1,
		kBTOpposite = 1 << 2,
		kFX = 1 << 3,
		kFXOpposite = 1 << 4,
		kLaser = 1 << 5,
		kLaserOpposite = 1 << 6,

		kArrowOrBT = kArrow | kBT,
		kArrowOrBTAll = kArrow | kBT | kBTOpposite,
		kArrowOrFX = kArrow | kFX,
		kArrowOrLaser = kArrow | kLaser,
		kArrowOrLaserAll = kArrow | kLaser | kLaserOpposite,
		kArrowOrBTOrLaser = kArrow | kBT | kLaser,
		kArrowOrBTAllOrLaserAll = kArrow | kBT | kBTOpposite | kLaser | kLaserOpposite,
		kArrowOrBTOrFXOrLaser = kArrow | kBT | kFX | kLaser,
	};
}

class CursorInput
{
private:
	ButtonCursorInputDevice m_buttonDevice;

public:
	enum class Type
	{
		Unspecified, // 未指定(※使用不可)
		Horizontal,  // 横向き
		Vertical,    // 縦向き
	};

	/// @brief CursorInputの生成パラメータ
	struct CreateInfo
	{
		/// @brief カーソル移動の種類(横向きor縦向き)
		CursorInput::Type type = Type::Unspecified;

		/// @brief 使用するボタンのビットフラグ
		/// @note 型がint32になっているのは、ビットフラグを|で重ねてint32になった場合にも、enumへのキャストの省略は許容したいため
		int32 buttonFlags = CursorButtonFlags::kNone;

		/// @brief カーソル移動の所要押下時間(0の場合、押下し続けても連続でカーソル移動しない)
		double buttonIntervalSec = 0.0;

		/// @brief 1回目のカーソル移動までの所要押下時間(0の場合、buttonIntervalSecと同じ値に設定される)
		double buttonIntervalSecFirst = 0.0;
	};

	/// @brief コンストラクタ
	explicit CursorInput(const CreateInfo& createInfo);

	~CursorInput() = default;

	/// @brief フレーム毎の更新(毎フレーム、deltaCursor使用前に呼ぶ)
	void update();

	/// @brief 現在フレームでのカーソル移動数を返す
	/// @return カーソル移動数(移動していない場合は0)
	int32 deltaCursor() const;
};
