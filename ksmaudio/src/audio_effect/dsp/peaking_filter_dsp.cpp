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

		// 余韻の長さ(秒)
		constexpr float kFilterReleaseSec = 0.05f;

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
		float GetPeakingFilterBaseGainDb(float v)
		{
			// HSP版での計算式を再現したもの:
			// https://github.com/m4saka/kshootmania-v1-hsp/blob/08275836547c7792a6d4f59037e56e947f2979c3/src/scene/play/play_audio_effects.hsp#L974

			const float baseGain = 14.0f * (std::min)(v, 0.15f) / 0.15f;
			const float midGain = 3.0f * (1.0f - std::abs(v - 0.35f) / 0.35f);
			constexpr float kHiGainCutThreshold = 0.8f;
			const float hiGainCut = (v > kHiGainCutThreshold) ? -3.5f * (v - kHiGainCutThreshold) / (1.0f - kHiGainCutThreshold) : 0.0f;
			return std::clamp(baseGain + midGain + hiGainCut, 0.0f, 18.0f) / 0.5f;
		}
	}

	namespace detail
	{
		PeakingFilterValueController::PeakingFilterValueController()
			: m_vEasing(kVEasingSpeed)
		{
		}

		void PeakingFilterValueController::updateGain(float v)
		{
			if (v != m_prevVInGainUpdate)
			{
				// gainの値はイージング適用前のものをもとに計算する
				// (直角時に一瞬低域が強調されてノイズが入るのを回避するため)
				m_baseGainDb = GetPeakingFilterBaseGainDb(v);
				m_prevVInGainUpdate = v;
				m_updated = true;
			}
		}

		void PeakingFilterValueController::updateFreq(float v)
		{
			// 値が飛ぶことでノイズが入らないようvの値に対して線形のイージングを入れる
			const bool valueUpdated = m_vEasing.update(v);
			if (valueUpdated)
			{
				const float easedV = m_vEasing.value();
				m_freq = GetPeakingFilterFreqValue(easedV);
				m_updated = true;
			}
		}

		bool PeakingFilterValueController::popUpdated()
		{
			return std::exchange(m_updated, false);
		}

		float PeakingFilterValueController::baseGainDb() const
		{
			return m_baseGainDb;
		}

		float PeakingFilterValueController::freq() const
		{
			return m_freq;
		}

		bool PeakingFilterValueController::mixSkipped() const
		{
			// 低周波数に対しては適用しない
			return m_freq < kFreqThresholdMin;
		}

		PeakingFilterRelease::PeakingFilterRelease(std::size_t sampleRate)
			: m_filterReleaseFrames(static_cast<std::size_t>(static_cast<float>(sampleRate) * kFilterReleaseSec))
			, m_mixSkippedFrames(m_filterReleaseFrames) // 最初に余韻が入らないよう閾値以上から始める
		{
		}

		void PeakingFilterRelease::update(float freq, float baseGainDb, float mix, bool mixSkipped)
		{
			if (mixSkipped)
			{
				++m_mixSkippedFrames;
			}
			else
			{
				m_freq = freq;
				m_baseGainDb = baseGainDb;
				m_mix = mix;
				m_mixSkippedFrames = 0U;
			}
		}

		bool PeakingFilterRelease::hasValue() const
		{
			return m_mixSkippedFrames < m_filterReleaseFrames;
		}

		float PeakingFilterRelease::freq() const
		{
			return m_freq;
		}

		float PeakingFilterRelease::baseGainDb() const
		{
			const float scale = std::clamp(1.0f - static_cast<float>(m_mixSkippedFrames) / m_filterReleaseFrames, 0.0f, 1.0f);
			return m_baseGainDb * scale;
		}

		float PeakingFilterRelease::mix() const
		{
			return m_mix;
		}
	}

	PeakingFilterDSP::PeakingFilterDSP(const DSPCommonInfo& info)
		: m_info(info)
		, m_release(info.sampleRate)
	{
	}

	void PeakingFilterDSP::process(float* pData, std::size_t dataSize, bool bypass, const PeakingFilterDSPParams& params)
	{
		if (m_info.isUnsupported)
		{
			return;
		}

		const float fSampleRate = static_cast<float>(m_info.sampleRate);
		const bool isBypassed = bypass || params.mix == 0.0f; // 切り替え時のノイズ回避のためにbypass状態でも処理自体はする

		assert(dataSize % m_info.numChannels == 0U);
		const std::size_t frameSize = dataSize / m_info.numChannels;
		m_valueController.updateGain(params.v);
		for (std::size_t i = 0U; i < frameSize; ++i)
		{
			m_valueController.updateFreq(params.v);
			const bool mixSkipped = isBypassed || m_valueController.mixSkipped();
			const bool shouldUseRelease = mixSkipped && params.releaseEnabled && m_release.hasValue();
			if (shouldUseRelease)
			{
				// 余韻を適用する必要がある場合はフィルタ係数を毎回更新
				for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
				{
					m_peakingFilters[ch].setPeakingFilter(m_release.freq(), params.bandwidth, m_release.baseGainDb() * params.gainRate, fSampleRate);
				}
			}
			else
			{
				const bool valueUpdated = m_valueController.popUpdated();
				if (valueUpdated)
				{
					// 値が更新された場合はフィルタ係数を更新
					for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
					{
						m_peakingFilters[ch].setPeakingFilter(m_valueController.freq(), params.bandwidth, m_valueController.baseGainDb() * params.gainRate, fSampleRate);
					}
				}
			}

			// 各チャンネルにフィルタを適用
			for (std::size_t ch = 0U; ch < m_info.numChannels; ++ch)
			{
				const float wet = m_peakingFilters[ch].process(*pData);
				if (shouldUseRelease)
				{
					*pData = std::lerp(*pData, wet, m_release.mix());
				}
				else if (!mixSkipped)
				{
					*pData = std::lerp(*pData, wet, params.mix);
				}
				++pData;
			}

			m_release.update(m_valueController.freq(), m_valueController.baseGainDb(), params.mix, mixSkipped);
		}
	}

	void PeakingFilterDSP::updateParams(const PeakingFilterDSPParams& params)
	{
		// 特に何もしない
	}
}
