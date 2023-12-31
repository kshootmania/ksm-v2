#include "auto_mute_addon.hpp"
#include "ksmaudio/ksmaudio.hpp"

AutoMuteAddon::AutoMuteAddon()
	: m_isMute(m_isEnabled && !Window::GetState().focused)
{
	ksmaudio::SetMute(m_isMute);
}

bool AutoMuteAddon::update()
{
	bool isMute = m_isEnabled && !Window::GetState().focused;
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
