#include "ksmaudio/stream.hpp"
#include <fstream>
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

	HSTREAM LoadStream(const std::string& filePath, const std::vector<char>* pPreloadedBinary)
	{
		if (pPreloadedBinary == nullptr)
		{
			return BASS_StreamCreateFile(FALSE, filePath.c_str(), 0, 0, BASS_STREAM_PRESCAN);
		}
		else
		{
			return BASS_StreamCreateFile(TRUE, pPreloadedBinary->data(), 0, static_cast<QWORD>(pPreloadedBinary->size()), BASS_STREAM_PRESCAN);
		}
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
	Stream::Stream(const std::string& filePath, double volume, bool enableCompressor, bool preload)
		: m_preloadedBinary(preload ? Preload(filePath) : nullptr)
		, m_hStream(LoadStream(filePath, m_preloadedBinary.get()))
		, m_info(GetChannelInfo(m_hStream))
	{
		// 音量を設定
		BASS_ChannelSetAttribute(m_hStream, BASS_ATTRIB_VOL, static_cast<float>(volume));

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

	double Stream::posSec() const
	{
		return BASS_ChannelBytes2Seconds(m_hStream, BASS_ChannelGetPosition(m_hStream, BASS_POS_BYTE));
	}

	void Stream::seekPosSec(double timeSec) const
	{
		BASS_ChannelSetPosition(m_hStream, BASS_ChannelSeconds2Bytes(m_hStream, timeSec), 0);
	}

	double Stream::durationSec() const
	{
		return BASS_ChannelBytes2Seconds(m_hStream, BASS_ChannelGetLength(m_hStream, BASS_POS_BYTE));
	}

	HDSP Stream::addAudioEffect(AudioEffect::IAudioEffect* pAudioEffect, int priority) const
	{
		return BASS_ChannelSetDSP(m_hStream, ProcessAudioEffect, pAudioEffect, priority);
	}

	void Stream::removeAudioEffect(HDSP hDSP) const
	{
		BASS_ChannelRemoveDSP(m_hStream, hDSP);
	}

	std::size_t Stream::sampleRate() const
	{
		return static_cast<std::size_t>(m_info.freq);
	}

	std::size_t Stream::numChannels() const
	{
		return static_cast<std::size_t>(m_info.chans);
	}

	double Stream::latencySec() const
	{
		/*DWORD playbuf = BASS_ChannelGetData(m_hStream, NULL, BASS_DATA_AVAILABLE);
		if (playbuf != (DWORD)-1)
		{
			return BASS_ChannelBytes2Seconds(m_hStream, playbuf);
		}*/

		// 上記でも大抵バッファサイズと同じになるが変動するので、そのままバッファサイズを返した方が音声エフェクトのタイミング計算が安定する
		return kBufferSizeMs / 1000.0f;
	}
}
