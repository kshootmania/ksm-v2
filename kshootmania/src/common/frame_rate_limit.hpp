#pragma once

class FrameRateLimit : public IAddon
{
private:
	int32 m_targetFPS;

	std::chrono::time_point<std::chrono::steady_clock> m_sleepUntil;

public:
	explicit FrameRateLimit(int32 targetFPS);

	virtual void postPresent() override;

	void setTargetFPS(int32 targetFPS);
};
