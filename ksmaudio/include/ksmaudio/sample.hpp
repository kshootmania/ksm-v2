#pragma once
#include <string>
#include "bass.h"

namespace ksmaudio
{
	class Sample
	{
	private:
		const HSAMPLE m_hSample;

	public:
		// Note: filePath must be in UTF-8
		Sample(const std::string& filePath, DWORD maxPolyphony = 1U);

		~Sample();

		Sample(const Sample&) = delete;

		Sample& operator=(const Sample&) = delete;

		Sample(Sample&&) = default;

		Sample& operator=(Sample&&) = default;

		void play(double volume = 1.0) const;
	};
}
