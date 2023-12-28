#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/sidechain_params.hpp"

namespace ksmaudio::AudioEffect
{
	class SidechainDSP
	{
	private:
		const DSPCommonInfo m_info;
		std::size_t m_cursorFrames = 0U;
		std::size_t m_attackFrames = 0U;
		std::size_t m_holdFrames = 0U;
		std::size_t m_releaseFrames = 0U;
		std::ptrdiff_t m_framesUntilTrigger = -1;

		float getScale() const;

	public:
		explicit SidechainDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const SidechainDSPParams& params);

		void updateParams(const SidechainDSPParams& params);
	};
}
