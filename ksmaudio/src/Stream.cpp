#include "ksmaudio/Stream.hpp"
#include <fstream>
#include <optional>
#include "ksmaudio/ksmaudio.hpp"

namespace
{
	// コンプレッサーのパラメータ値
	// HSP版: https://github.com/m4saka/kshootmania-v1-hsp/blob/461901f1e925cb8cb474fd02726084cfca9ec3d4/kshootmania.hsp#L792
	// (音割れ改善のためにHSP版から変更している)
	constexpr BASS_BFX_COMPRESSOR2 kCompressorFXParams
	{
		.fGain = 18.0f,
		.fThreshold = -24.0f,
		.fRatio = 5.0f,
		.fAttack = 1.0f, // ※ms単位(44サンプル)
		.fRelease = 20.0f, // ※ms単位(882サンプル)
		.lChannel = BASS_BFX_CHANALL,
	};

	// コンプレッサーの前段の音量変更のパラメータ値
	constexpr BASS_BFX_VOLUME kVolumeFXParams
	{
		.lChannel = BASS_BFX_CHANALL,
		.fVolume = 0.1f,
	};

	// 各エフェクトの優先順位
	constexpr int kCompressorFXPriority = 0;
	constexpr int kVolumeAmplifyFXPriority = 5;
	constexpr int kVolumeFXPriority = 10;

	std::unique_ptr<std::vector<char>> Preload(const std::string& filePath)
	{
		std::ifstream ifs(filePath, std::ios::in | std::ios::binary);
		if (!ifs)
		{
			return nullptr;
		}
		ifs.seekg(0, std::ios::end);
		const std::size_t fileSize = static_cast<std::size_t>(ifs.tellg());
		auto binary = std::make_unique<std::vector<char>>(fileSize);
		ifs.seekg(0, std::ios::beg);
		ifs.read(binary->data(), fileSize);
		return binary;
	}

	HSTREAM LoadStream(const std::string& filePath, const std::vector<char>* pPreloadedBinary, bool loop, bool forTempo)
	{
		const DWORD loopFlag = loop ? BASS_SAMPLE_LOOP : 0;
		const DWORD decodeFlag = forTempo ? BASS_STREAM_DECODE : 0;
		if (pPreloadedBinary == nullptr)
		{
			return BASS_StreamCreateFile(FALSE, filePath.c_str(), 0, 0, BASS_STREAM_PRESCAN | loopFlag | decodeFlag);
		}
		else
		{
			return BASS_StreamCreateFile(TRUE, pPreloadedBinary->data(), 0, static_cast<QWORD>(pPreloadedBinary->size()), BASS_STREAM_PRESCAN | loopFlag | decodeFlag);
		}
	}

	HSTREAM CreateTempoStream(HSTREAM hSourceStream, double playbackSpeed)
	{
		HSTREAM hTempoStream = BASS_FX_TempoCreate(hSourceStream, 0);
		const double tempo = (playbackSpeed - 1.0) * 100.0;
		BASS_ChannelSetAttribute(hTempoStream, BASS_ATTRIB_TEMPO, static_cast<float>(tempo));
		return hTempoStream;
	}

	BASS_CHANNELINFO GetChannelInfo(HSTREAM hStream)
	{
		BASS_CHANNELINFO info;
		BASS_ChannelGetInfo(hStream, &info);
		return info;
	}

	void ProcessAudioEffect(HDSP handle, DWORD channel, void* buffer, DWORD length, void* user)
	{
		const auto pAudioEffect = reinterpret_cast<ksmaudio::AudioEffect::IAudioEffect*>(user);
		const auto pData = reinterpret_cast<float*>(buffer);
		pAudioEffect->process(pData, length / sizeof(float));
	}
}

namespace ksmaudio
{
	namespace
	{
		std::optional<HSTREAM> CreateSourceStream(const std::vector<char>* pPreloadedBinary, const std::string& filePath, bool loop, double playbackSpeed)
		{
			if (playbackSpeed != 1.0)
			{
				return LoadStream(filePath, pPreloadedBinary, loop, true);
			}
			return std::nullopt;
		}

		HSTREAM CreateMainStream(std::optional<HSTREAM> hStreamSource, const std::vector<char>* pPreloadedBinary, const std::string& filePath, bool loop, double playbackSpeed)
		{
			if (hStreamSource.has_value())
			{
				return CreateTempoStream(hStreamSource.value(), playbackSpeed);
			}
			return LoadStream(filePath, pPreloadedBinary, loop, false);
		}
	}

	Stream::Stream(const std::string& filePath, double volume, bool enableCompressor, bool preload, bool loop, double playbackSpeed)
		: m_preloadedBinary(preload ? Preload(filePath) : nullptr)
		, m_hStreamSource(CreateSourceStream(m_preloadedBinary.get(), filePath, loop, playbackSpeed))
		, m_hStream(CreateMainStream(m_hStreamSource, m_preloadedBinary.get(), filePath, loop, playbackSpeed))
		, m_playbackSpeed(playbackSpeed)
		, m_info(GetChannelInfo(m_hStream))
		, m_volume(volume)
		, m_muted(false)
	{
		// 音量を設定
		applyVolume();

		// コンプレッサーを適用
		if (enableCompressor)
		{
			const HFX hCompressorFX = BASS_ChannelSetFX(m_hStream, BASS_FX_BFX_COMPRESSOR2, kCompressorFXPriority);
			BASS_FXSetParameters(hCompressorFX, &kCompressorFXParams);

			const HFX hVolumeFX = BASS_ChannelSetFX(m_hStream, BASS_FX_BFX_VOLUME, kVolumeFXPriority);
			BASS_FXSetParameters(hVolumeFX, &kVolumeFXParams);
		}
	}

	Stream::~Stream()
	{
		BASS_StreamFree(m_hStream);
		if (m_hStreamSource.has_value())
		{
			BASS_StreamFree(m_hStreamSource.value());
		}
	}

	void Stream::applyVolume()
	{
		if (m_volume > 1.0)
		{
			// 100%超の場合はBASS_FX_BFX_VOLUMEエフェクトで増幅
			if (m_hVolumeAmplifyFX == 0)
			{
				m_hVolumeAmplifyFX = BASS_ChannelSetFX(m_hStream, BASS_FX_BFX_VOLUME, kVolumeAmplifyFXPriority);
			}
			const BASS_BFX_VOLUME params{ .lChannel = BASS_BFX_CHANALL, .fVolume = static_cast<float>(m_volume) };
			BASS_FXSetParameters(m_hVolumeAmplifyFX, &params);
			BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_VOL, 1.0f);
		}
		else
		{
			// 100%以下の場合はBASS_ATTRIB_VOLを使用
			if (m_hVolumeAmplifyFX != 0)
			{
				// エフェクトを無効化
				const BASS_BFX_VOLUME params{ .lChannel = BASS_BFX_CHANALL, .fVolume = 1.0f };
				BASS_FXSetParameters(m_hVolumeAmplifyFX, &params);
			}
			BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_VOL, static_cast<float>(m_volume));
		}
	}

	void Stream::play() const
	{
		BASS_ChannelPlay(m_hStream, FALSE);
	}

	void Stream::pause() const
	{
		BASS_ChannelPause(m_hStream);
	}

	void Stream::stop() const
	{
		BASS_ChannelStop(m_hStream);
	}

	void Stream::updateManually() const
	{
		BASS_ChannelUpdate(m_hStream, 0);
	}

	SecondsF Stream::posSec() const
	{
		if (m_playbackSpeed == 0.0)
		{
			return SecondsF{ 0.0 };
		}
		const double scaledTime = BASS_ChannelBytes2Seconds(m_hStream, BASS_ChannelGetPosition(m_hStream, BASS_POS_BYTE));
		return SecondsF{ scaledTime / m_playbackSpeed };
	}

	void Stream::seekPosSec(SecondsF time) const
	{
		const double scaledTime = time.count() * m_playbackSpeed;
		BASS_ChannelSetPosition(m_hStream, BASS_ChannelSeconds2Bytes(m_hStream, scaledTime), BASS_POS_BYTE);
	}

	Duration Stream::duration() const
	{
		if (m_playbackSpeed == 0.0)
		{
			return Duration{ 0.0 };
		}
		const double scaledDuration = BASS_ChannelBytes2Seconds(m_hStream, BASS_ChannelGetLength(m_hStream, BASS_POS_BYTE));
		return Duration{ scaledDuration / m_playbackSpeed };
	}

	HDSP Stream::addAudioEffect(AudioEffect::IAudioEffect* pAudioEffect, int priority) const
	{
		return BASS_ChannelSetDSP(m_hStream, ProcessAudioEffect, pAudioEffect, priority);
	}

	void Stream::removeAudioEffect(HDSP hDSP) const
	{
		BASS_ChannelRemoveDSP(m_hStream, hDSP);
	}

	void Stream::setFadeIn(Duration duration) const
	{
		// フェード係数を0から1まで推移させる
		BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_VOL, 0.0f);
		BASS_ChannelSlideAttribute(m_hStream, BASS_ATTRIB_VOL, 1.0f, static_cast<DWORD>(duration.count() * 1000));
	}

	void Stream::setFadeIn(Duration duration, double volume)
	{
		m_volume = volume;
		applyVolume();
		setFadeIn(duration);
	}

	void Stream::setFadeOut(Duration duration) const
	{
		// フェード係数を1から0まで推移させる
		BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_VOL, 1.0f);
		BASS_ChannelSlideAttribute(m_hStream, BASS_ATTRIB_VOL, 0.0f, static_cast<DWORD>(duration.count() * 1000));
	}

	void Stream::setFadeOut(Duration duration, double volume)
	{
		m_volume = volume;
		applyVolume();
		setFadeOut(duration);
	}
	
	bool Stream::isPlaying() const
	{
		const DWORD status = BASS_ChannelIsActive(m_hStream);
		return status == BASS_ACTIVE_PLAYING || status == BASS_ACTIVE_STALLED;
	}

	bool Stream::isFading() const
	{
		return BASS_ChannelIsSliding(m_hStream, BASS_ATTRIB_VOL);
	}

	void Stream::setVolume(double volume)
	{
		m_volume = volume;
		if (!m_muted)
		{
			applyVolume();
		}
	}

	double Stream::volume() const
	{
		return m_volume;
	}

	bool Stream::muted() const
	{
		return m_muted;
	}

	void Stream::setMuted(bool muted)
	{
		m_muted = muted;
		if (muted)
		{
			BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_VOL, 0.0f);
		}
		else
		{
			// 100%超の場合はフェード係数1.0、100%以下の場合はm_volume
			const float attribVol = m_volume > 1.0 ? 1.0f : static_cast<float>(m_volume);
			BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_VOL, attribVol);
		}
	}

	std::size_t Stream::sampleRate() const
	{
		return static_cast<std::size_t>(m_info.freq);
	}

	std::size_t Stream::numChannels() const
	{
		return static_cast<std::size_t>(m_info.chans);
	}

	SecondsF Stream::latency() const
	{
#if defined(_WIN32)
		// Windowsの場合はBASS_DATA_AVAILABLEで取得される値の変動が大きいため、バッファサイズを定数で返した方が音声エフェクトのタイミング計算が安定する
		return SecondsF{ kBufferSizeMs / 1000.0f };
#else
		// Linux/macOSの場合はBASS_DATA_AVAILABLEで取得される値がバッファサイズと異なるため、取得したものを返す
		DWORD playbuf = BASS_ChannelGetData(m_hStream, NULL, BASS_DATA_AVAILABLE);
		if (playbuf != (DWORD)-1)
		{
			return SecondsF{ BASS_ChannelBytes2Seconds(m_hStream, playbuf) };
		}
		return SecondsF{ kBufferSizeMs / 1000.0f };
#endif
	}

	void Stream::lockBegin() const
	{
		BASS_ChannelLock(m_hStream, TRUE);
	}

	void Stream::lockEnd() const
	{
		BASS_ChannelLock(m_hStream, FALSE);
	}
}
