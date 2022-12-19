#include "ksmaudio/audio_effect/dsp/low_pass_filter_dsp.hpp"

namespace ksmaudio::AudioEffect
{
	namespace
	{
		// 1フレーム(1/44100秒)あたりのvの線形イージングの速さ(最大変化量)
		constexpr float kVEasingSpeed = 0.01f;

		// フィルタ適用可能な最低周波数
		// (高周波を強調すると耳障りになるためしきい値を設けている)
		constexpr float kFreqThresholdMax = 14800.0f;

		// TODO: freq、freq_maxの値を変更可能にする
		float GetLowPassFilterFreqValue(float v)
		{
			// HSP版での計算式を再現したもの:
			// https://github.com/m4saka/kshootmania-v1-hsp/blob/08275836547c7792a6d4f59037e56e947f2979c3/src/scene/play/play_audio_effects.hsp#L957

			return 15000.0f - 14200.0f * std::sin(std::sin(v * 1.25f) / std::sin(1.25f) * 1.5f) / std::sin(1.5f);
		}
	}

	LowPassFilterDSP::LowPassFilterDSP(const DSPCommonInfo& info)
		: m_info(info)
		, m_vEasing(kVEasingSpeed)
	{
	}

	void LowPassFilterDSP::process(float* pData, std::size_t dataSize, bool bypass, const LowPassFilterDSPParams& params, bool isParamUpdated)
	{
		if (m_info.isUnsupported)
		{
			return;
		}

		const float fSampleRate = static_cast<float>(m_info.sampleRate);
		const float targetFreq = GetLowPassFilterFreqValue(params.v);
		const bool isBypassed = bypass || params.mix == 0.0f; // 切り替え時のノイズ回避のためにbypass状態でも処理自体はする

		assert(dataSize % m_info.numChannels == 0U);
		const std::size_t frameSize = dataSize / m_info.numChannels;
		float freq = GetLowPassFilterFreqValue(m_vEasing.value());
		bool mixSkipped = isBypassed || freq > kFreqThresholdMax; // 高周波数に対しては適用しない
		for (std::size_t i = 0U; i < frameSize; ++i)
		{
			// 値が飛ぶことでノイズが入らないようvの値に対して線形のイージングを入れる
			const bool vUpdated = m_vEasing.update(params.v);
			if (vUpdated)
			{
				freq = GetLowPassFilterFreqValue(m_vEasing.value());
				mixSkipped = isBypassed || freq > kFreqThresholdMax; // 高周波数に対しては適用しない
			}

			// 各チャンネルにフィルタを適用
			for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
			{
				if (vUpdated)
				{
					m_lowPassFilters[ch].setLowPassFilter(freq, params.q, fSampleRate);
				}

				const float wet = m_lowPassFilters[ch].process(*pData);
				if (!mixSkipped)
				{
					*pData = std::lerp(*pData, wet, params.mix);
				}
				++pData;
			}
		}
	}
}
