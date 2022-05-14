#include "ksmaudio/audio_effect/audio_effect_param.hpp"
#include <string>
#include <algorithm>
#include <stdexcept>
#include <cmath>

namespace ksmaudio::AudioEffect
{
	// Implementation in HSP: https://github.com/m4saka/kshootmania-v1-hsp/blob/19bfb6acbec8abd304b2e7dae6009df8e8e1f66f/src/scene/play/play_utils.hsp#L405
	float StrToValue(Type type, const std::string& str)
	{
		try
		{
			switch (type)
			{
			case Type::kLength:
				// value
				//   > 0: measure
				//   < 0: sec
				if (str.ends_with("ms"))
				{
					return -std::max(std::stof(str), 0.0f) / 1000;
				}
				else if (str.starts_with("1/"))
				{
					const int d = std::stoi(str.substr(2)); // 2 = strlen("1/")
					if (d > 0)
					{
						return 1.0f / d;
					}
					else
					{
						return 0.0f;
					}
				}
				else if (str.ends_with('s') && !str.ends_with("es")) // Do not allow "XXXsamples"
				{
					return -std::max(std::stof(str), 0.0f);
				}
				// TODO: more validations needed here
				return std::max(std::stof(str), 0.0f);

			case Type::kSample:
				if (str.ends_with("samples"))
				{
					return std::clamp(std::stof(str), 0.0f, 44100.0f);
				}
				break;

			case Type::kSwitch:
				if (str == "on")
				{
					return 1.0f;
				}
				return 0.0f;

			case Type::kRate:
				if (str.ends_with('%'))
				{
					return std::clamp(std::stof(str) / 100, 0.0f, 1.0f);
				}
				return std::clamp(std::stof(str), 0.0f, 1.0f);

			case Type::kFreq:
				if (str.ends_with("kHz"))
				{
					return std::max(std::stof(str), 0.0f) * 1000;
				}
				else if (str.ends_with("Hz"))
				{
					return std::max(std::stof(str), 0.0f);
				}
				return 0.0f;

			case Type::kDB:
				if (str.ends_with("dB"))
				{
					return std::stof(str);
				}
				return 0.0f;

			case Type::kPitch:
				// param
				//   > 0: not quantized (real_value + 48)
				//   < 0: quantized (-real_value - 48)
				{
					const float value = std::stof(str);
					if ((value >= -48.0f) && (value <= 48.0f))
					{
						if (str.contains('.'))
						{
							return value + 48.0f;
						}
						else
						{
							return -(value + 48.0f);
						}
					}
				}
				return 0.0f;

			case Type::kInt:
				return static_cast<float>(std::stoi(str));

			case Type::kFloat:
				return std::stof(str);

			case Type::kFilename:
				return 0.0f;

			default:
				// Just ignore errors here
				return 0.0f;
			}
		}
		catch ([[maybe_unused]] const std::invalid_argument& e)
		{
			// Just ignore errors here
			return 0.0f;
		}
		catch ([[maybe_unused]] const std::out_of_range& e)
		{
			// Just ignore errors here
			return 0.0f;
		}
	}

	ValueSet StrToValueSet(Type type, const std::string& str, bool* pSuccess)
	{
		const std::size_t pos1 = str.find('>');
		const std::size_t pos2 = str.find('-', pos1 + 2); // 2 = strlen(">" + negative value sign "-")

		const std::string offStr = str.substr(0, pos1);
		const std::string onMinStr = (pos1 == std::string::npos) ? offStr : str.substr(pos1 + 1, pos2);
		const std::string onMaxStr = (pos2 == std::string::npos) ? onMinStr : str.substr(pos2 + 1);

		ValueSet valueSet = {
			.off = StrToValue(type, offStr),
			.onMin = StrToValue(type, onMinStr),
			.onMax = StrToValue(type, onMaxStr),
		};

		// For length parameters, the min and max values must have the same sign.
		// Otherwise, a value set of 0 is returned.
		if (type == Type::kLength && ((valueSet.onMin < 0) != (valueSet.onMax < 0)))
		{
			if (pSuccess != nullptr)
			{
				*pSuccess = false;
			}
			return {};
		}

		if (pSuccess != nullptr)
		{
			*pSuccess = true;
		}

		// For pitch parameters, the min and max values must have the same sign.
		// Otherwise, quantization is disabled (i.e., "0.0-12" is replaced by "0.0-12.0").
		if (type == Type::kPitch && ((valueSet.onMin < 0) != (valueSet.onMax < 0)))
		{
			return {
				.off = valueSet.off,
				.onMin = std::abs(valueSet.onMin),
				.onMax = std::abs(valueSet.onMax),
			};
		}

		return valueSet;
	}

	float GetValue(const Param& param, const Status& status)
	{
		const float lerped = status.isOn ? std::lerp(param.valueSet.onMin, param.valueSet.onMax, status.v) : param.valueSet.off;

		if (param.type == Type::kLength)
		{
			if (lerped > 0.0f)
			{
				// Tempo-synced
				return lerped * 4 * 60 / status.bpm;
			}
			else
			{
				// Not tempo-synced
				return -lerped;
			}
		}
		
		if (param.type == Type::kPitch)
		{
			if (lerped > 0.0f)
			{
				// Not quantized
				return lerped - 48.0f;
			}
			else
			{
				// Quantized
				return std::floor(-lerped - 48.0f);
			}
		}

		return lerped;
	}

	Param DefineParam(Type type, const std::string& valueSetStr)
	{
		return {
			.type = type,
			.valueSet = StrToValueSet(type, valueSetStr),
		};
	}
}
