#include "ksmaudio/audio_effect/dsp/tapestop_dsp.hpp"
#include <cmath>

namespace ksmaudio::AudioEffect
{
	namespace
	{
		// 1�t���[��������̍Đ����x�̌�����
		// TODO: pow�̌v�Z�R�X�g�����������m��Ȃ��B�e�[�u��������H
		float SlowDownFactor(float speedParam, std::size_t framesSinceTrigger, float sampleRateScale)
		{
			const float scale = (std::max)(speedParam * 100, 0.0f) + 1.0f;
			const float rate = std::pow(2.5f, -static_cast<float>(framesSinceTrigger) / 8000.0f / sampleRateScale) + 0.5f;
			return scale * rate / 500000.0f;
		}

		// �Đ����x�����ƂɃt�F�[�h�A�E�g�p�̐U���{�������߂�
		constexpr float FadeOutScale(float playSpeed)
		{
			if (0.0f <= playSpeed && playSpeed < 0.25f)
			{
				return playSpeed * 4.0f;
			}

			return 1.0f;
		}

		// �f�B���C�^�C���̍ő�b��(�����O�o�b�t�@�̃T�C�Y)
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

	void TapestopDSP::process(float* pData, std::size_t dataSize, bool bypass, const TapestopDSPParams& params)
	{
		if (m_info.isUnsupported)
		{
			return;
		}

		assert(dataSize % m_info.numChannels == 0);

		if (!bypass && params.trigger && params.mix > 0.0f)
		{
			const std::size_t frameSize = dataSize / m_info.numChannels;
			for (std::size_t i = 0U; i < frameSize; ++i)
			{
				const float playSpeed = m_speedController.nextSpeed(params, m_info.sampleRateScale);
				m_timeModulator.update(playSpeed);
				for (std::size_t ch = 0; ch < m_info.numChannels; ++ch)
				{
					const float wet = m_timeModulator.readWrite(*pData, ch);
					*pData = std::lerp(*pData, wet, params.mix);
					++pData;
				}
				m_timeModulator.advanceCursor();
			}
		}
		else
		{
			m_speedController.reset();
			m_timeModulator.writeAndAdvanceCursor(pData, dataSize);
		}
	}
}