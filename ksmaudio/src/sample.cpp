#include "ksmaudio/sample.hpp"
#include <cassert>

namespace
{
	HSAMPLE LoadSample(const std::string& filePath, DWORD maxPolyphony)
	{
		assert(1 <= maxPolyphony && maxPolyphony <= 65535);
		return BASS_SampleLoad(FALSE, filePath.c_str(), 0, 0, maxPolyphony, BASS_SAMPLE_OVER_POS);
	}
}

namespace ksmaudio
{
	Sample::Sample(const std::string& filePath, DWORD maxPolyphony)
		: m_hSample(LoadSample(filePath, maxPolyphony))
	{
	}

	Sample::~Sample()
	{
		BASS_SampleFree(m_hSample);
	}

	void Sample::play(double volume) const
	{
		const HCHANNEL hChannel = BASS_SampleGetChannel(m_hSample, FALSE);
		BASS_ChannelSetAttribute(hChannel, BASS_ATTRIB_VOL, static_cast<float>(volume));
		BASS_ChannelPlay(hChannel, TRUE);
	}
}
