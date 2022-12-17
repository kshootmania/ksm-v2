#include "ksmaudio/audio_effect/dsp/peaking_filter_dsp.hpp"
#include <algorithm>

namespace ksmaudio::AudioEffect
{
	namespace
	{
		// 1フレーム(1/44100秒)あたりのvの線形イージングの速さ(最大変化量)
		constexpr float kVEasingSpeed = 0.01f;

		// フィルタ適用可能な最低周波数
		// (低周波を強調すると波形の振幅が過剰に大きくなるためしきい値を設けている)
		constexpr float kFreqThresholdMin = 100.0f;

		// TODO: freq、freq_maxの値を変更可能にする
		float GetPeakingFilterFreqValue(float v)
		{
			// HSP版での計算式を再現したもの:
			// https://github.com/m4saka/kshootmania-v1-hsp/blob/08275836547c7792a6d4f59037e56e947f2979c3/src/scene/play/play_audio_effects.hsp#L974

			const float cosV = std::cos(v);
			const float cos2V = cosV * cosV;
			const float baseFreq = 10000.0f - 9950.0f * (cosV * 4 + cos2V * 5) / 9;

			constexpr float kSpeedUpThreshold = 0.8f;
			const float speedUpFreq = (v > kSpeedUpThreshold) ? 3000.0f * (v - kSpeedUpThreshold) / (1.0f - kSpeedUpThreshold) : 0.0f;

			return baseFreq + speedUpFreq;
		}

		// TODO: freq、freq_maxの値を加味する
		float GetPeakingFilterGainDb(float v, float gainRate)
		{
			// HSP版での計算式を再現したもの:
			// https://github.com/m4saka/kshootmania-v1-hsp/blob/08275836547c7792a6d4f59037e56e947f2979c3/src/scene/play/play_audio_effects.hsp#L974

			const float baseGain = 14.0f * (std::min)(v, 0.15f) / 0.15f;
			const float midGain = 3.0f * (1.0f - std::abs(v - 0.35f) / 0.35f);
			constexpr float kHiGainCutThreshold = 0.8f;
			const float hiGainCut = (v > kHiGainCutThreshold) ? -3.5f * (v - kHiGainCutThreshold) / (1.0f - kHiGainCutThreshold) : 0.0f;
			return std::clamp(baseGain + midGain + hiGainCut, 0.0f, 18.0f) * gainRate / 0.5f;
		}
	}

	PeakingFilterDSP::PeakingFilterDSP(const DSPCommonInfo& info)
		: m_info(info)
		, m_vEasing(kVEasingSpeed)
	{
	}

	void PeakingFilterDSP::process(float* pData, std::size_t dataSize, bool bypass, const PeakingFilterDSPParams& params)
	{
		if (m_info.isUnsupported)
		{
			return;
		}

		const float fSampleRate = static_cast<float>(m_info.sampleRate);
		const float targetFreq = GetPeakingFilterFreqValue(params.v);
		const float gainDb = GetPeakingFilterGainDb(params.v, params.gainRate);
		const bool isBypassed = bypass || params.mix == 0.0f; // 切り替え時のノイズ回避のためにbypass状態でも処理自体はする

		assert(dataSize % m_info.numChannels == 0U);
		const std::size_t frameSize = dataSize / m_info.numChannels;
		float freq = GetPeakingFilterFreqValue(m_vEasing.value());
		bool mixSkipped = isBypassed || freq < kFreqThresholdMin; // 低周波数に対しては適用しない
		for (std::size_t i = 0U; i < frameSize; ++i)
		{
			// 値が飛ぶことでノイズが入らないようvの値に対して線形のイージングを入れる
			const bool vUpdated = m_vEasing.update(params.v);
			if (vUpdated)
			{
				freq = GetPeakingFilterFreqValue(m_vEasing.value());
				mixSkipped = isBypassed || freq < kFreqThresholdMin; // 低周波数に対しては適用しない
			}

			// 各チャンネルにフィルタを適用
			for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
			{
				if (vUpdated)
				{
					m_peakingFilters[ch].setPeakingFilter(freq, params.bandwidth, gainDb, fSampleRate);
				}

				const float wet = m_peakingFilters[ch].process(*pData);
				if (!mixSkipped)
				{
					*pData = std::lerp(*pData, wet, params.mix);
				}
				++pData;
			}
		}
	}
}
