#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/bitcrusher_params.hpp"

namespace ksmaudio::AudioEffect
{
	class BitcrusherDSP
	{
	private:
		const DSPCommonInfo m_info;
		std::array<float, 2> m_holdSample = { 0.0f, 0.0f };
		float m_holdSampleCount = 0.0f;

	public:
		explicit BitcrusherDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const BitcrusherDSPParams& params);

		void updateParams(const BitcrusherDSPParams& params);
	};
}
