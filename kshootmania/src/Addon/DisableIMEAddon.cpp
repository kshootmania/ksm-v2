#include "DisableIMEAddon.hpp"

#if defined(_WIN32) || defined(__APPLE__)

#include "Common/IMEUtils.hpp"
#include "Ini/ConfigIni.hpp"

namespace
{
	constexpr double kDisableIMELowIntervalSec = 0.1;
	constexpr double kDisableIMEMidIntervalSec = 0.05;
	constexpr double kDisableIMEHighIntervalSec = 0.01;

	constexpr double GetDisableIMEIntervalSec(DisableIMEMode mode)
	{
		switch (mode)
		{
		case DisableIMEMode::kLow:
			return kDisableIMELowIntervalSec;
		case DisableIMEMode::kMid:
			return kDisableIMEMidIntervalSec;
		case DisableIMEMode::kHigh:
			return kDisableIMEHighIntervalSec;
		default:
			return 0.0;
		}
	}
}

DisableIMEAddon::DisableIMEAddon()
	: m_mode(static_cast<DisableIMEMode>(ConfigIni::GetInt(ConfigIni::Key::kDisableIME, static_cast<int32>(DisableIMEMode::kHigh))))
{
	// 初回実行
	if (m_mode != DisableIMEMode::kOff)
	{
		IMEUtils::DetachIMEContext();

		m_detachStopwatch.restart();
	}
}

bool DisableIMEAddon::update()
{
	// ウィンドウが非アクティブの場合は処理しない
	if (!Window::GetState().focused)
	{
		if (m_detachStopwatch.isStarted())
		{
			m_detachStopwatch.reset();
		}
		m_wasEditingTextBox = false;

		return true;
	}

	if (m_mode == DisableIMEMode::kOff)
	{
		if (m_detachStopwatch.isStarted())
		{
			m_detachStopwatch.reset();
		}
		m_wasEditingTextBox = false;

		return true;
	}

	// テキストボックス編集中はIME無効化しない
	const bool isEditing = noco::IsEditingTextBox();
	if (isEditing)
	{
		if (!m_wasEditingTextBox)
		{
			IMEUtils::AttachIMEContext();
		}
		m_wasEditingTextBox = true;
		if (m_detachStopwatch.isStarted())
		{
			m_detachStopwatch.reset();
		}
		return true;
	}

	// テキストボックス編集終了時はIME無効化を再開
	if (m_wasEditingTextBox)
	{
		IMEUtils::DetachIMEContext();
		m_wasEditingTextBox = false;
		m_detachStopwatch.restart();
		return true;
	}

	const double intervalSec = GetDisableIMEIntervalSec(m_mode);
	if (!m_detachStopwatch.isStarted())
	{
		m_detachStopwatch.restart();
	}

	if (m_detachStopwatch.sF() >= intervalSec)
	{
		IMEUtils::DetachIMEContext();
		m_detachStopwatch.restart();
	}

	return true;
}

void DisableIMEAddon::SetMode(DisableIMEMode mode)
{
	if (const auto pAddon = Addon::GetAddon<DisableIMEAddon>(kAddonName))
	{
		pAddon->m_mode = mode;

		if (mode == DisableIMEMode::kOff)
		{
			IMEUtils::AttachIMEContext();
			pAddon->m_detachStopwatch.reset();
			pAddon->m_wasEditingTextBox = false;
		}
		else
		{
			pAddon->m_detachStopwatch.restart();
		}
	}
}

#endif
