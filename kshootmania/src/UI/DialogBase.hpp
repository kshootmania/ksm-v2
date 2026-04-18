#pragma once

/// @brief ダイアログの基底クラス(Layer::ModalでCo::HasActiveModal()による判定が可能)
template <typename TResult = void>
class DialogBase : public Co::SequenceBase<TResult>
{
public:
	DialogBase()
		: Co::SequenceBase<TResult>(Co::Layer::Modal)
	{
	}
};
