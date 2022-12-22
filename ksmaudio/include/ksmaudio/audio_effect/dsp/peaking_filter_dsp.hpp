#pragma once
#include <optional>
#include "ksmaudio/audio_effect/audio_effect.hpp"
#include "ksmaudio/audio_effect/params/peaking_filter_params.hpp"
#include "ksmaudio/audio_effect/detail/biquad_filter.hpp"
#include "ksmaudio/audio_effect/detail/linear_easing.hpp"

namespace ksmaudio::AudioEffect
{
	namespace detail
	{
		class PeakingFilterValueController
		{
		private:
			detail::LinearEasing<float> m_vEasing;
			std::optional<float> m_prevVInGainUpdate = std::nullopt;
			float m_baseGainDb = 0.0f;
			float m_freq = 1000.0f;
			bool m_updated = true;

		public:
			PeakingFilterValueController();

			void updateGain(float v);

			void updateFreq(float v);

			bool popUpdated();

			float baseGainDb() const;

			float freq() const;

			bool mixSkipped() const;
		};

		// PeakingFilterの余韻
		// フィルタが無効になってから50msの間は、無効になる直前の値を使用してgainを減少させつつフィルタ適用する
		// LASERノーツ直後に"シュワ"という音を出すこと、およびLASER判定が一瞬外れた際にもフィルタが途切れないことを目的としている
		//
		// HSP版の該当箇所: https://github.com/m4saka/kshootmania-v1-hsp/blob/1c75880b545d1232eeffc4bb3fc19704a3622f73/src/scene/play/play_audio_effects.hsp#L901
		// (HSP版では余韻のロジックを使い回して直角判定後にgainを増幅する操作も入っているが、効果が薄いことと処理が複雑になるためC++版では廃止)
		class PeakingFilterRelease
		{
		private:
			const std::size_t m_filterReleaseFrames;
			float m_freq = 1000.0f;
			float m_baseGainDb = 0.0f;
			float m_mix = 0.0f;
			std::size_t m_mixSkippedFrames;

		public:
			explicit PeakingFilterRelease(std::size_t sampleRate);

			void update(float freq, float baseGainDb, float mix, bool mixSkipped);

			bool hasValue() const;

			float freq() const;

			float baseGainDb() const;

			float mix() const;
		};
	}

	class PeakingFilterDSP
	{
	private:
		const DSPCommonInfo m_info;
		std::array<detail::BiquadFilter<float>, 2> m_peakingFilters;
		detail::PeakingFilterValueController m_valueController;
		detail::PeakingFilterRelease m_release; // TODO: FXでは使用しないようにする

	public:
		explicit PeakingFilterDSP(const DSPCommonInfo& info);

		void process(float* pData, std::size_t dataSize, bool bypass, const PeakingFilterDSPParams& params);

		void updateParams(const PeakingFilterDSPParams& params);
	};
}
