#include "ksmaudio/audio_effect/dsp/sidechain_dsp.hpp"

namespace ksmaudio::AudioEffect
{
	float SidechainDSP::getScale() const
	{
		if (m_cursorFrames < m_attackFrames)
		{
			return 1.0f - static_cast<float>(m_cursorFrames) / m_attackFrames;
		}
		else if (m_cursorFrames < m_attackFrames + m_holdFrames)
		{
			return 0.0f;
		}
		else if (m_cursorFrames < m_attackFrames + m_holdFrames + m_releaseFrames)
		{
			return static_cast<float>(m_cursorFrames - (m_attackFrames + m_holdFrames)) / m_releaseFrames;
		}
		else
		{
			return 1.0f;
		}
	}

	SidechainDSP::SidechainDSP(const DSPCommonInfo& info)
		: m_info(info)
	{
	}

	void SidechainDSP::process(float* pData, std::size_t dataSize, bool bypass, const SidechainDSPParams& params)
	{
		if (m_info.isUnsupported || bypass)
		{
			return;
		}

		assert(dataSize % m_info.numChannels == 0);
		const std::size_t numFrames = dataSize / m_info.numChannels;
		for (std::size_t i = 0; i < numFrames; ++i)
		{
			if (m_framesUntilTrigger >= 0)
			{
				--m_framesUntilTrigger;
				if (m_framesUntilTrigger < 0)
				{
					// トリガ更新
					m_cursorFrames = 0;
					m_attackFrames = static_cast<std::size_t>(params.attackTime * m_info.sampleRate);
					m_holdFrames = static_cast<std::size_t>(params.holdTime * m_info.sampleRate);
					m_releaseFrames = static_cast<std::size_t>(params.releaseTime * m_info.sampleRate);
					m_framesUntilTrigger = -1;
				}
			}

			const float scale = getScale();
			const float scaleWithRatio = std::lerp(scale, 1.0f, 1.0f / params.ratio);
			for (std::size_t channel = 0; channel < m_info.numChannels; ++channel)
			{
				*pData *= scaleWithRatio;
				++pData;
			}
			++m_cursorFrames;
		}
	}

	void SidechainDSP::updateParams(const SidechainDSPParams& params)
	{
		// トリガ更新までのフレーム数を計算
		if (params.secUntilTrigger >= 0.0f) // 負の値は無視
		{
			const std::ptrdiff_t newFramesUntilTrigger = static_cast<std::ptrdiff_t>(params.secUntilTrigger * m_info.sampleRate);

			// 前回より小さい場合のみ反映(トリガ発生寸前に次の時間が入ることでトリガが抜ける現象を回避するため)
			if (m_framesUntilTrigger < 0 || m_framesUntilTrigger > newFramesUntilTrigger)
			{
				m_framesUntilTrigger = newFramesUntilTrigger;
			}
		}
	}
}
