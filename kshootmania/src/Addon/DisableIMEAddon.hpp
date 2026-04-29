#pragma once

#if defined(_WIN32) || defined(__APPLE__)

enum class DisableIMEMode : int32
{
	kOff = 0,
	kLow = 1,
	kMid = 2,
	kHigh = 3,
};

class DisableIMEAddon : public IAddon
{
public:
	static constexpr StringView kAddonName = U"DisableIME";

private:
	DisableIMEMode m_mode = DisableIMEMode::kHigh;
	Stopwatch m_detachStopwatch{ StartImmediately::No };
	bool m_wasEditingTextBox = false;

public:
	DisableIMEAddon();

	virtual ~DisableIMEAddon() = default;

	virtual bool update() override;

	static void SetMode(DisableIMEMode mode);
};
#endif
