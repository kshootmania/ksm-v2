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

	bool ValueAsBool(float value);

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

	bool GetValueAsBool(const Param& param, const Status& status, bool isOn);

	Param DefineParam(Type type, const std::string& valueSetStr);

	enum class ParamID
	{
		kUnknown,
		kUpdatePeriod,
		kWaveLength,
		kRate,
		kUpdateTrigger,
		kMix,
		kPeriod,
		kDelay,
		kDepth,
		kFeedback,
		kStereoWidth,
		kVol,
		kPitch,
		kChunkSize,
		kOverlap,
		kReduction,
		kStage,
		kLoFreq,
		kHiFreq,
		kQ,
		kSpeed,
		kTrigger,
		kFeedbackLevel,
		kHoldTime,
		kAttackTime,
		kReleaseTime,
		kRatio,
		kFilename,
		kV,
		kFreq,
		kFreqMax,
		kGain,
	};

	inline const std::unordered_map<std::string, ParamID> kStrToParamID{
		{ "update_period", ParamID::kUpdatePeriod },
		{ "wave_length", ParamID::kWaveLength },
		{ "rate", ParamID::kRate },
		{ "update_trigger", ParamID::kUpdateTrigger },
		{ "mix", ParamID::kMix },
		{ "period", ParamID::kPeriod },
		{ "delay", ParamID::kDelay },
		{ "depth", ParamID::kDepth },
		{ "feedback", ParamID::kFeedback },
		{ "stereo_width", ParamID::kStereoWidth },
		{ "vol", ParamID::kVol },
		{ "pitch", ParamID::kPitch },
		{ "chunk_size", ParamID::kChunkSize },
		{ "overlap", ParamID::kOverlap },
		{ "reduction", ParamID::kReduction },
		{ "stage", ParamID::kStage },
		{ "lo_freq", ParamID::kLoFreq },
		{ "hi_freq", ParamID::kHiFreq },
		{ "q", ParamID::kQ },
		{ "speed", ParamID::kSpeed },
		{ "trigger", ParamID::kTrigger },
		{ "feedback_level", ParamID::kFeedbackLevel },
		{ "hold_time", ParamID::kHoldTime },
		{ "attack_time", ParamID::kAttackTime },
		{ "release_time", ParamID::kReleaseTime },
		{ "ratio", ParamID::kRatio },
		{ "filename", ParamID::kFilename },
		{ "v", ParamID::kV },
		{ "freq", ParamID::kFreq },
		{ "freq_max", ParamID::kFreqMax },
		{ "gain", ParamID::kGain },
	};

	inline const std::unordered_map<ParamID, Type> kParamIDType{
		{ ParamID::kUnknown, Type::kUnspecified },
		{ ParamID::kUpdatePeriod, Type::kLength },
		{ ParamID::kWaveLength, Type::kWaveLength },
		{ ParamID::kRate, Type::kRate },
		{ ParamID::kUpdateTrigger, Type::kSwitch },
		{ ParamID::kMix, Type::kRate },
		{ ParamID::kPeriod, Type::kLength },
		{ ParamID::kDelay, Type::kSample },
		{ ParamID::kDepth, Type::kSample },
		{ ParamID::kFeedback, Type::kRate },
		{ ParamID::kStereoWidth, Type::kRate },
		{ ParamID::kVol, Type::kRate },
		{ ParamID::kPitch, Type::kPitch },
		{ ParamID::kChunkSize, Type::kSample },
		{ ParamID::kOverlap, Type::kRate },
		{ ParamID::kReduction, Type::kSample },
		{ ParamID::kStage, Type::kInt },
		{ ParamID::kLoFreq, Type::kFreq },
		{ ParamID::kHiFreq, Type::kFreq },
		{ ParamID::kQ, Type::kFloat },
		{ ParamID::kSpeed, Type::kRate },
		{ ParamID::kTrigger, Type::kSwitch },
		{ ParamID::kFeedbackLevel, Type::kRate },
		{ ParamID::kHoldTime, Type::kLength },
		{ ParamID::kAttackTime, Type::kLength },
		{ ParamID::kReleaseTime, Type::kLength },
		{ ParamID::kRatio, Type::kInt },
		{ ParamID::kFilename, Type::kFilename },
		{ ParamID::kV, Type::kRate },
		{ ParamID::kFreq, Type::kFreq },
		{ ParamID::kFreqMax, Type::kFreq },
		{ ParamID::kGain, Type::kRate },
	};

	using ParamValueSetDict = std::unordered_map<ParamID, ValueSet>;
}
