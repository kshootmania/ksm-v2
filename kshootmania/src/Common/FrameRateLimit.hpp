#pragma once

class FrameRateLimit : public IAddon
{
private:
	Optional<int32> m_targetFPS;

	std::chrono::time_point<std::chrono::steady_clock> m_sleepUntil;

public:
	static constexpr StringView kAddonName = U"FrameRateLimit";

	explicit FrameRateLimit(const Optional<int32>& targetFPS);

	virtual void postPresent() override;

	void setTargetFPS(const Optional<int32>& targetFPS) noexcept;
};
