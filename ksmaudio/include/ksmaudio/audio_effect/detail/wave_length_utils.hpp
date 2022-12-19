#pragma once

namespace ksmaudio::AudioEffect::detail::WaveLengthUtils
{
	int DenominatorToQuantizationArrayIdx(int d);

	float Interpolate(float a, float b, float rate);
}
