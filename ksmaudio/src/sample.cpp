#include "ksmaudio/sample.hpp"
#include <cassert>

namespace
{
	HSAMPLE LoadSample(const std::string& filePath, DWORD maxPolyphony)
	{
		assert(1 <= maxPolyphony && maxPolyphony <= 65535);
		return BASS_SampleLoad(FALSE, filePath.c_str(), 0, 0, maxPolyphony, 0);
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

	void Sample::play() const
	{
		const HCHANNEL hChannel = BASS_SampleGetChannel(m_hSample, BASS_SAMPLE_OVER_POS);
		BASS_ChannelPlay(hChannel, FALSE);
	}

}
