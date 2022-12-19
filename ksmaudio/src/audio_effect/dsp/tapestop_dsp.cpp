#include "ksmaudio/audio_effect/dsp/tapestop_dsp.hpp"
#include <cmath>

namespace ksmaudio::AudioEffect
{
	namespace
	{
		// 1フレームあたりの再生速度の減衰量
		// TODO: powの計算コストが高いかも知れない。テーブル化する？
		float SlowDownFactor(float speedParam, std::size_t framesSinceTrigger, float sampleRateScale)
		{
			const float scale = (std::max)(speedParam * 100, 0.0f) + 1.0f;
			const float rate = std::pow(2.5f, -static_cast<float>(framesSinceTrigger) / 8000.0f / sampleRateScale) + 0.5f;
			return scale * rate / 500000.0f;
		}

		// 再生速度をもとにフェードアウト用の振幅倍率を求める
		constexpr float FadeOutScale(float playSpeed)
		{
			if (0.0f <= playSpeed && playSpeed < 0.25f)
			{
				return playSpeed * 4.0f;
			}

			return 1.0f;
		}

		// ディレイタイムの最大秒数(リングバッファのサイズ)
		constexpr std::size_t kMaxDelayTimeSec = 10U;
	}

	namespace detail
	{
		float TapeStopSpeedController::nextSpeed(const TapestopDSPParams& params, float sampleRateScale)
		{
			if (params.trigger)
			{
				m_currentPlaySpeed -= SlowDownFactor(params.speed, m_framesSinceTrigger, sampleRateScale);
				if (m_currentPlaySpeed < 0.0f)
				{
					m_currentPlaySpeed = 0.0f;
				}
				++m_framesSinceTrigger;
			}
			else
			{
				reset();
			}

			return m_currentPlaySpeed;
		}

		void TapeStopSpeedController::reset()
		{
			m_currentPlaySpeed = 1.0f;
			m_framesSinceTrigger = 0U;
		}
	}

	TapestopDSP::TapestopDSP(const DSPCommonInfo& info)
		: m_info(info)
		, m_timeModulator(kMaxDelayTimeSec * info.sampleRate * info.numChannels, info.numChannels)
	{
	}

	void TapestopDSP::process(float* pData, std::size_t dataSize, bool bypass, const TapestopDSPParams& params, bool isParamUpdated)
	{
		if (m_info.isUnsupported)
		{
			return;
		}

		assert(dataSize % m_info.numChannels == 0);
		
		if (params.reset && isParamUpdated) // resetの値はパラメータ更新後の初回実行時のみ有効
		{
			m_timeModulator.resetDelayTime();
			m_speedController.reset();
		}

		if (!bypass && params.trigger && params.mix > 0.0f)
		{
			const std::size_t frameSize = dataSize / m_info.numChannels;
			for (std::size_t i = 0U; i < frameSize; ++i)
			{
				const float playSpeed = m_speedController.nextSpeed(params, m_info.sampleRateScale);
				const float fadeOutScale = FadeOutScale(playSpeed);
				m_timeModulator.update(playSpeed);
				for (std::size_t ch = 0; ch < m_info.numChannels; ++ch)
				{
					const float wet = m_timeModulator.readWrite(*pData, ch) * fadeOutScale;
					*pData = std::lerp(*pData, wet, params.mix);
					++pData;
				}
				m_timeModulator.advanceCursor();
			}
		}
		else
		{
			m_timeModulator.writeAndAdvanceCursor(pData, dataSize);
		}
	}
}
