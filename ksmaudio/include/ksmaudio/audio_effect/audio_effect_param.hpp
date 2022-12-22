#pragma once
#include <string>
#include <set>
#include <optional>
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

	class TapestopTriggerParam
	{
	private:
		Param m_innerParam;

		bool m_prevTrigger = false;

		std::optional<std::size_t> m_prevLaneIdx = std::nullopt;

		bool m_reset = false;

	public:
		explicit TapestopTriggerParam(const ValueSet& valueSet)
			: m_innerParam{ .type = Type::kSwitch, .valueSet = valueSet }
		{
		}

		bool renderByFX(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();

			// DSPパラメータ上のtrigger自体は通常のパラメータと同じ
			const bool trigger = GetValueAsBool(m_innerParam, status, isOn);

			// triggerがfalseからtrueに変わったとき、または
			// ノーツ中に別のレーンのノーツを追加で押してupdateTriggerがoff→onになったときは、resetの値をtrueにする
			m_reset = trigger && (!m_prevTrigger || laneIdx != m_prevLaneIdx);

			m_prevTrigger = trigger;
			m_prevLaneIdx = laneIdx;

			return trigger;
		}

		bool renderByLaser(const Status& status, bool isOn)
		{
			// DSPパラメータ上のtrigger自体は通常のパラメータと同じ
			const bool trigger = GetValueAsBool(m_innerParam, status, isOn);

			// triggerがfalseからtrueに変わったときは、resetの値をtrueにする
			m_reset = trigger && !m_prevTrigger;

			m_prevTrigger = trigger;
			m_prevLaneIdx = std::nullopt;

			return trigger;
		}

		bool getResetValue() const
		{
			return m_reset;
		}

		Param* innerParamPtr()
		{
			return &m_innerParam;
		}
	};

	struct UpdateTriggerParam
	{
	private:
		Param m_innerParam;

		bool m_prevRawUpdateTrigger = false;

		std::optional<std::size_t> m_prevLaneIdx = std::nullopt;

	public:
		explicit UpdateTriggerParam(const ValueSet& valueSet)
			: m_innerParam{ .type = Type::kSwitch, .valueSet = valueSet }
		{
		}

		bool renderByFX(const Status& status, std::optional<std::size_t> laneIdx)
		{
			const bool isOn = laneIdx.has_value();

			// updateTriggerの"Off>OnMin-OnMax"のOffのトリガタイミングは事前に計算済みで別途secUntilTrigger側で処理されるため無視する
			// (ただし、"on>off-on"や"on-off"の場合はプレイ中にoff→onになりうるので無視せず、3つすべて"on"の場合のみ無視する。secUntilTriggerの方と多重に更新される場合もあることになるが実用上大した問題はない)
			const bool ignoreUpdateTrigger = m_innerParam.valueSet.off && m_innerParam.valueSet.onMin && m_innerParam.valueSet.onMax;

			// DSPパラメータ上のupdateTriggerはoff→onに変わった瞬間だけtrueにする
			const bool rawUpdateTrigger = GetValueAsBool(m_innerParam, status, isOn) && !ignoreUpdateTrigger;
			const bool updateTriggerValue = rawUpdateTrigger && (!m_prevRawUpdateTrigger || laneIdx != m_prevLaneIdx); // ノーツ中に別のレーンのノーツを追加で押してupdateTriggerがoff→onになる場合もあるので、laneIdxの変化もOR条件に入れる
			m_prevRawUpdateTrigger = rawUpdateTrigger;
			m_prevLaneIdx = laneIdx;

			return updateTriggerValue;
		}

		bool renderByLaser(const Status& status, bool isOn)
		{
			// updateTriggerの"Off>OnMin-OnMax"のOffのトリガタイミングは事前に計算済みで別途secUntilTrigger側で処理されるため無視する
			// (ただし、"on>off-on"や"on-off"の場合はプレイ中にoff→onになりうるので無視せず、3つすべて"on"の場合のみ無視する。secUntilTriggerの方と多重に更新される場合もあることになるが実用上大した問題はない)
			const bool ignoreUpdateTrigger = m_innerParam.valueSet.off && m_innerParam.valueSet.onMin && m_innerParam.valueSet.onMax;

			// DSPパラメータ上のupdateTriggerはoff→onに変わった瞬間だけtrueにする
			const bool rawUpdateTrigger = GetValueAsBool(m_innerParam, status, isOn) && !ignoreUpdateTrigger;
			const bool updateTriggerValue = rawUpdateTrigger && !m_prevRawUpdateTrigger;
			m_prevRawUpdateTrigger = rawUpdateTrigger;
			m_prevLaneIdx = std::nullopt;

			return updateTriggerValue;
		}

		Param* innerParamPtr()
		{
			return &m_innerParam;
		}
	};

	TapestopTriggerParam DefineTapestopTriggerParam(const std::string& valueSetStr);

	UpdateTriggerParam DefineUpdateTriggerParam(const std::string& valueSetStr);

	// パラメータのID
	// (実行時の管理にしか使用しないので、後から末尾以外へ列挙子を追加しても問題ない)
	enum class ParamID
	{
		kUnknown,
		kAttackTime,
		kBandwidth,
		kDelay,
		kDepth,
		kFeedback,
		kFeedbackLevel,
		kFilename,
		kFreq,
		kFreq1,
		kFreq2,
		kFreqMax,
		kGain,
		kHoldTime,
		kMix,
		kPeriod,
		kPitch,
		kQ,
		kRate,
		kRatio,
		kReduction,
		kReleaseTime,
		kSpeed,
		kStage,
		kStereoWidth,
		kTrigger,
		kUpdateTrigger,
		kV,
		kVol,
		kWaveLength,
		kUpdatePeriod,
	};

	inline const std::unordered_map<std::string, ParamID> kStrToParamID{
		{ "attack_time", ParamID::kAttackTime },
		{ "delay", ParamID::kDelay },
		{ "depth", ParamID::kDepth },
		{ "feedback_level", ParamID::kFeedbackLevel },
		{ "feedback", ParamID::kFeedback },
		{ "filename", ParamID::kFilename },
		{ "freq", ParamID::kFreq },
		{ "freq_1", ParamID::kFreq1 },
		{ "freq_2", ParamID::kFreq2 },
		{ "freq_max", ParamID::kFreqMax },
		{ "gain", ParamID::kGain },
		{ "hold_time", ParamID::kHoldTime },
		{ "mix", ParamID::kMix },
		{ "period", ParamID::kPeriod },
		{ "pitch", ParamID::kPitch },
		{ "q", ParamID::kQ },
		{ "rate", ParamID::kRate },
		{ "ratio", ParamID::kRatio },
		{ "reduction", ParamID::kReduction },
		{ "release_time", ParamID::kReleaseTime },
		{ "speed", ParamID::kSpeed },
		{ "stage", ParamID::kStage },
		{ "stereo_width", ParamID::kStereoWidth },
		{ "trigger", ParamID::kTrigger },
		{ "update_trigger", ParamID::kUpdateTrigger },
		{ "v", ParamID::kV },
		{ "vol", ParamID::kVol },
		{ "wave_length", ParamID::kWaveLength },
		{ "update_period", ParamID::kUpdatePeriod },
	};

	inline const std::unordered_map<ParamID, Type> kParamIDType{
		{ ParamID::kUnknown, Type::kUnspecified },
		{ ParamID::kAttackTime, Type::kLength },
		{ ParamID::kDelay, Type::kSample },
		{ ParamID::kDepth, Type::kSample },
		{ ParamID::kFeedback, Type::kRate },
		{ ParamID::kFeedbackLevel, Type::kRate },
		{ ParamID::kFilename, Type::kFilename },
		{ ParamID::kFreq, Type::kFreq },
		{ ParamID::kFreq1, Type::kFreq },
		{ ParamID::kFreq2, Type::kFreq },
		{ ParamID::kFreqMax, Type::kFreq },
		{ ParamID::kGain, Type::kRate },
		{ ParamID::kHoldTime, Type::kLength },
		{ ParamID::kMix, Type::kRate },
		{ ParamID::kPeriod, Type::kLength },
		{ ParamID::kPitch, Type::kPitch },
		{ ParamID::kQ, Type::kFloat },
		{ ParamID::kRate, Type::kRate },
		{ ParamID::kRatio, Type::kInt },
		{ ParamID::kReduction, Type::kSample },
		{ ParamID::kReleaseTime, Type::kLength },
		{ ParamID::kSpeed, Type::kRate },
		{ ParamID::kStage, Type::kInt },
		{ ParamID::kStereoWidth, Type::kRate },
		{ ParamID::kTrigger, Type::kSwitch },
		{ ParamID::kUpdateTrigger, Type::kSwitch },
		{ ParamID::kV, Type::kRate },
		{ ParamID::kVol, Type::kRate },
		{ ParamID::kWaveLength, Type::kWaveLength },
		{ ParamID::kUpdatePeriod, Type::kLength },
	};

	using ParamValueSetDict = std::unordered_map<ParamID, ValueSet>;
}
