#include "ksmaudio/stream.hpp"

namespace
{
	HSTREAM LoadStream(const std::string& filePath)
	{
		return BASS_StreamCreateFile(FALSE, filePath.c_str(), 0, 0, BASS_SAMPLE_FLOAT | BASS_STREAM_PRESCAN);
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

	Stream::Stream(const std::string& filePath)
		: m_hStream(LoadStream(filePath))
		, m_info(GetChannelInfo(m_hStream))
	{
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
		DWORD playbuf = BASS_ChannelGetData(m_hStream, NULL, BASS_DATA_AVAILABLE);
		if (playbuf != (DWORD)-1)
		{
			return BASS_ChannelBytes2Seconds(m_hStream, playbuf);
		}
		return 0.0; // TODO: return kBufferSizeMs
	}

}
