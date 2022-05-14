#pragma once
#include <string>
#include <unordered_map>

namespace ksmaudio::AudioEffect
{
	enum class Type
	{
		kUnspecified,
		kLength,
		kSample,
		kSwitch,
		kRate,
		kFreq,
		kDB,
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
		bool isOn = false;

		float v = 0.0f;

		float bpm = 120.0f;
	};

	float GetValue(const Param& param, const Status& status);

	Param DefineParam(Type type, const std::string& valueSetStr);
}
