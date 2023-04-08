#include "frame_rate_limit.hpp"

FrameRateLimit::FrameRateLimit(int32 targetFPS)
	: m_targetFPS(targetFPS)
	, m_sleepUntil(std::chrono::steady_clock::now())
{
}

void FrameRateLimit::postPresent()
{
	// 次フレームまでのsleepの終了時間を決める
	m_sleepUntil += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)) / m_targetFPS;

	// 目標フレームレートに届かなかったときにm_sleepUntilが現在時間より過去に離れていかないよう現在時間以降にする
	const auto now = std::chrono::steady_clock::now();
	if (m_sleepUntil < now)
	{
		m_sleepUntil = now;
	}

	// sleepを実行
	std::this_thread::sleep_until(m_sleepUntil);
}

void FrameRateLimit::setTargetFPS(int32 targetFPS)
{
	m_targetFPS = targetFPS;
}
