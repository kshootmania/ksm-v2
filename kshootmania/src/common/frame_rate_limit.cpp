#include "frame_rate_limit.hpp"

namespace
{
	constexpr std::chrono::steady_clock::duration kMaxDrift = 10ms;
}

FrameRateLimit::FrameRateLimit(int32 targetFPS)
	: m_targetFPS(targetFPS)
	, m_sleepUntil(std::chrono::steady_clock::now() - kMaxDrift)
{
}

void FrameRateLimit::postPresent()
{
	// 次フレームまでのsleepの終了時間を決める
	m_sleepUntil += std::chrono::duration_cast<std::chrono::steady_clock::duration>(1s) / m_targetFPS;

	// 目標フレームレートに届かなかったときにm_sleepUntilが現在時間より過去に離れていかないよう下限を設ける
	// (環境によっては現在時刻ちょうどまでのsleepでも長めの時間待ちが発生する場合があるため、対策のためにkMaxDriftを引いた時間を下限にしている)
	const auto sleepUntilLowerLimit = std::chrono::steady_clock::now() - kMaxDrift;
	if (m_sleepUntil < sleepUntilLowerLimit)
	{
		m_sleepUntil = sleepUntilLowerLimit;
	}

	// sleepを実行
	std::this_thread::sleep_until(m_sleepUntil);
}

void FrameRateLimit::setTargetFPS(int32 targetFPS)
{
	m_targetFPS = targetFPS;
}
