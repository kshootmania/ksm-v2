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
		Sample(const std::string& filePath, DWORD maxPolyphony);

		~Sample();

		void play() const;
	};

}
