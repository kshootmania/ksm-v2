#pragma once
#include <string>

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

	struct ParamDef
	{
		std::string name;

		Type type = Type::kUnspecified;
	};

	float StrToValue(Type type, const std::string& str);

	ValueSet StrToValueSet(Type type, const std::string& str);

	struct Param
	{
		const ParamDef def;

		ValueSet valueSet;
	};

	struct Status
	{
		bool isOn = false;

		float v = 0.0f;

		float bpm = 120.0f;
	};

	float GetValue(Type type, const ValueSet& valueSet, const Status& status);

	float GetValue(const Param& param, const Status& status);

	Param DefineParam(const std::string& name, Type type, const std::string& valueSetStr);
}
