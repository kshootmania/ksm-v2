#pragma once
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/retrigger_echo_params.hpp"
#include "ksmaudio/audio_effect/detail/linear_buffer.hpp"

namespace ksmaudio::AudioEffect
{
	class RetriggerEchoDSP
	{
	private:
		const DSPCommonInfo m_info;
		detail::LinearBuffer<float> m_linearBuffer;
		std::ptrdiff_t m_framesUntilTrigger = -1;

	public:
		explicit RetriggerEchoDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const RetriggerEchoDSPParams& params);
	};
}
