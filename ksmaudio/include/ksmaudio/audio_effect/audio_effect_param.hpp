#pragma once
#include <string>
#include <set>
#include <unordered_map>

namespace ksmaudio::AudioEffect
{
	enum class Type
	{
		kUnspecified,
		kLength,
		kWaveLength,
		kSample,
		kSwitch,
		kRate,
		kFreq,
		kPitch,
		kInt,
		kFloat,
		kFilename,
	};

	struct ValueSet
	{
		float off = 0.0f;

		float onMin = 0.0f;

		float onMax = 0.0f;
	};

	float StrToValue(Type type, const std::string& str);

	ValueSet StrToValueSet(Type type, const std::string& str, bool* pIsError = nullptr);

	struct Param
	{
		Type type = Type::kUnspecified;

		ValueSet valueSet;
	};

	struct Status
	{
		float v = 0.0f; // Laser value (0-1)

		float bpm = 120.0f;

		float sec = -100.0f; // Non-zero negative value is used here to avoid update_trigger being processed before playback
	};

	float GetValue(const Param& param, const Status& status, bool isOn);

	Param DefineParam(Type type, const std::string& valueSetStr);
}
