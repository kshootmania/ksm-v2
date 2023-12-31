#pragma once

class AutoMuteAddon : public IAddon
{
public:
	static constexpr StringView kAddonName = U"AutoMute";

private:
	bool m_isEnabled = true;

	bool m_isMute;

public:
	AutoMuteAddon();

	virtual ~AutoMuteAddon() = default;

	virtual bool update() override;

	static void SetEnabled(bool isEnabled);
};
