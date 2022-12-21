#include "ksmaudio/audio_effect/dsp/high_pass_filter_dsp.hpp"

namespace ksmaudio::AudioEffect
{
	namespace
	{
		// 1フレーム(1/44100秒)あたりのvの線形イージングの速さ(最大変化量)
		constexpr float kVEasingSpeed = 0.01f;

		// フィルタ適用可能な最低周波数
		// (低周波を強調すると波形の振幅が過剰に大きくなるためしきい値を設けている)
		constexpr float kFreqThresholdMin = 200.0f;

		// TODO: freq、freq_maxの値を変更可能にする
		float GetHighPassFilterFreqValue(float v)
		{
			// HSP版での計算式を再現したもの:
			// https://github.com/m4saka/kshootmania-v1-hsp/blob/08275836547c7792a6d4f59037e56e947f2979c3/src/scene/play/play_audio_effects.hsp#L955

			return 4600.0f - 4500.0f * std::cos(v);
		}
	}

	HighPassFilterDSP::HighPassFilterDSP(const DSPCommonInfo& info)
		: m_info(info)
		, m_vEasing(kVEasingSpeed)
	{
	}

	void HighPassFilterDSP::process(float* pData, std::size_t dataSize, bool bypass, const HighPassFilterDSPParams& params)
	{
		if (m_info.isUnsupported)
		{
			return;
		}

		const float fSampleRate = static_cast<float>(m_info.sampleRate);
		const bool isBypassed = bypass || params.mix == 0.0f; // 切り替え時のノイズ回避のためにbypass状態でも処理自体はする

		assert(dataSize % m_info.numChannels == 0U);
		const std::size_t frameSize = dataSize / m_info.numChannels;
		float freq = GetHighPassFilterFreqValue(m_vEasing.value());
		bool mixSkipped = isBypassed || freq < kFreqThresholdMin; // 低周波数に対しては適用しない
		for (std::size_t i = 0U; i < frameSize; ++i)
		{
			// 値が飛ぶことでノイズが入らないようvの値に対して線形のイージングを入れる
			const bool vUpdated = m_vEasing.update(params.v);
			if (vUpdated)
			{
				freq = GetHighPassFilterFreqValue(m_vEasing.value());
				mixSkipped = isBypassed || freq < kFreqThresholdMin; // 低周波数に対しては適用しない
			}

			// 各チャンネルにフィルタを適用
			for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
			{
				if (vUpdated)
				{
					m_highPassFilters[ch].setHighPassFilter(freq, params.q, fSampleRate);
				}

				const float wet = m_highPassFilters[ch].process(*pData);
				if (!mixSkipped)
				{
					*pData = std::lerp(*pData, wet, params.mix);
				}
				++pData;
			}
		}
	}

	void HighPassFilterDSP::updateParams(const HighPassFilterDSPParams& params)
	{
		// 特に何もしない
	}
}
