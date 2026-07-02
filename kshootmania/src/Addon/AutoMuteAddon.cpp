#include "AutoMuteAddon.hpp"
#include "ksmaudio/ksmaudio.hpp"

namespace
{
	/// @brief ミュートすべきかどうかを返す(Web版ではフォーカス取得不可のため常にfalse)
	bool IsMuteRequired(bool isEnabled)
	{
#ifdef __EMSCRIPTEN__
		// emscriptenのGLFW実装はGLFW_FOCUSEDが常にfalseを返すため自動ミュートしない
		(void)isEnabled;
		return false;
#else
		return isEnabled && !Window::GetState().focused;
#endif
	}
}

AutoMuteAddon::AutoMuteAddon()
	: m_isMute(IsMuteRequired(m_isEnabled))
{
	ksmaudio::SetMute(m_isMute);
}

bool AutoMuteAddon::update()
{
	bool isMute = IsMuteRequired(m_isEnabled);
	if (m_isMute != isMute)
	{
		m_isMute = isMute;
		ksmaudio::SetMute(m_isMute);
	}

	return true;
}

void AutoMuteAddon::SetEnabled(bool isEnabled)
{
	if (const auto pAddon = Addon::GetAddon<AutoMuteAddon>(kAddonName))
	{
		pAddon->m_isEnabled = isEnabled;
	}
}
