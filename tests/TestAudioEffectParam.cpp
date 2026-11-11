#include <catch2/catch.hpp>
#include "ksmaudio/AudioEffect/AudioEffectParam.hpp"
#include "ksmaudio/AudioEffect/AudioEffectParamSpec.hpp"
#include "ksmaudio/AudioEffect/AudioEffectParamValidator.hpp"
#include "ksmaudio/AudioEffect/ParamController.hpp"
#include "ksmaudio/AudioEffect/Params/BitcrusherParams.hpp"
#include "ksmaudio/AudioEffect/Params/FlangerParams.hpp"
#include "ksmaudio/AudioEffect/Params/GateParams.hpp"
#include "ksmaudio/AudioEffect/Params/HighPassFilterParams.hpp"
#include "ksmaudio/AudioEffect/Params/LowPassFilterParams.hpp"
#include "ksmaudio/AudioEffect/Params/PeakingFilterParams.hpp"
#include "ksmaudio/AudioEffect/Params/PhaserParams.hpp"
#include "ksmaudio/AudioEffect/Params/PitchShiftParams.hpp"
#include "ksmaudio/AudioEffect/Params/RetriggerEchoParams.hpp"
#include "ksmaudio/AudioEffect/Params/SidechainParams.hpp"
#include "ksmaudio/AudioEffect/Params/TapestopParams.hpp"
#include "ksmaudio/AudioEffect/Params/WobbleParams.hpp"
#include "ksmaudio/AudioEffect/detail/BiquadFilter.hpp"
#include <cmath>
#include <utility>

using namespace ksmaudio::AudioEffect;

TEST_CASE("Parse length type parameters", "[AudioEffect][Param][Length]")
{
	bool success = false;

	// 分数形式(テンポ同期)
	success = false;
	REQUIRE(StrToValue(Type::kLength, "1/4", &success) == 0.25f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kLength, "1/2", &success) == 0.5f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kLength, "1/1", &success) == 1.0f);
	REQUIRE(success == true);

	// 小数形式(テンポ同期)
	success = false;
	REQUIRE(StrToValue(Type::kLength, "2.0", &success) == 2.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kLength, "0.25", &success) == 0.25f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kLength, "0", &success) == 0.0f);
	REQUIRE(success == true);

	// ミリ秒形式(非テンポ同期、区別のため負の値として扱う)
	success = false;
	REQUIRE(StrToValue(Type::kLength, "100ms", &success) == -0.1f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kLength, "10.5ms", &success) == Approx(-0.0105f));
	REQUIRE(success == true);

	// 秒形式(非テンポ同期、区別のため負の値として扱う)
	success = false;
	REQUIRE(StrToValue(Type::kLength, "1s", &success) == -1.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kLength, "0.1s", &success) == Approx(-0.1f));
	REQUIRE(success == true);

	// 範囲外の値は丸める
	success = false;
	REQUIRE(StrToValue(Type::kLength, "-1.0", &success) == 0.0f);
	REQUIRE(success == true);

	// 無効な文字列
	success = true;
	REQUIRE(StrToValue(Type::kLength, "10invalidms", &success) == 0.0f);
	REQUIRE(success == false);
	success = true;
	REQUIRE(StrToValue(Type::kLength, "10invalids", &success) == 0.0f);
	REQUIRE(success == false);
	success = true;
	REQUIRE(StrToValue(Type::kLength, "1/0", &success) == 0.0f);
	REQUIRE(success == false);
}

TEST_CASE("Parse sample type parameters", "[AudioEffect][Param][Sample]")
{
	bool success = false;

	// samples形式
	success = false;
	REQUIRE(StrToValue(Type::kSample, "40samples", &success) == 40.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kSample, "0samples", &success) == 0.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kSample, "44100samples", &success) == 44100.0f);
	REQUIRE(success == true);

	// 範囲外の値は丸める
	success = false;
	REQUIRE(StrToValue(Type::kSample, "50000samples", &success) == 44100.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kSample, "-100samples", &success) == 0.0f);
	REQUIRE(success == true);

	// 1の場合も"samples"しか受け付けない
	success = true;
	REQUIRE(StrToValue(Type::kSample, "1sample", &success) == 0.0f);
	REQUIRE(success == false);
}

TEST_CASE("Parse switch type parameters", "[AudioEffect][Param][Switch]")
{
	bool success = false;

	// on/off
	success = false;
	REQUIRE(StrToValue(Type::kSwitch, "on", &success) == 1.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kSwitch, "off", &success) == 0.0f);
	REQUIRE(success == true);
}

TEST_CASE("Parse rate type parameters", "[AudioEffect][Param][Rate]")
{
	bool success = false;

	// 分数形式
	success = false;
	REQUIRE(StrToValue(Type::kRate, "1/2", &success) == 0.5f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kRate, "1/4", &success) == 0.25f);
	REQUIRE(success == true);

	// パーセント形式
	success = false;
	REQUIRE(StrToValue(Type::kRate, "50%", &success) == 0.5f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kRate, "0%", &success) == 0.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kRate, "100%", &success) == 1.0f);
	REQUIRE(success == true);

	// 小数形式
	success = false;
	REQUIRE(StrToValue(Type::kRate, "0.5", &success) == 0.5f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kRate, "0.0", &success) == 0.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kRate, "1.0", &success) == 1.0f);
	REQUIRE(success == true);

	// 範囲外の値は丸める
	success = false;
	REQUIRE(StrToValue(Type::kRate, "150%", &success) == 1.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kRate, "-10%", &success) == 0.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kRate, "1.5", &success) == 1.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kRate, "-0.5", &success) == 0.0f);
	REQUIRE(success == true);

	// 無効な文字列
	success = true;
	REQUIRE(StrToValue(Type::kRate, "50invalid%", &success) == 0.0f);
	REQUIRE(success == false);
	success = true;
	REQUIRE(StrToValue(Type::kRate, "abc", &success) == 0.0f);
	REQUIRE(success == false);
	success = true;
	REQUIRE(StrToValue(Type::kRate, "", &success) == 0.0f);
	REQUIRE(success == false);
}

TEST_CASE("Parse freq type parameters", "[AudioEffect][Param][Freq]")
{
	bool success = false;

	// Hz形式
	success = false;
	REQUIRE(StrToValue(Type::kFreq, "1500Hz", &success) == 1500.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kFreq, "10Hz", &success) == 10.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kFreq, "20000Hz", &success) == 20000.0f);
	REQUIRE(success == true);

	// kHz形式
	success = false;
	REQUIRE(StrToValue(Type::kFreq, "1.5kHz", &success) == 1500.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kFreq, "0.01kHz", &success) == 10.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kFreq, "20.0kHz", &success) == 20000.0f);
	REQUIRE(success == true);

	// 範囲外の値は丸める
	success = false;
	REQUIRE(StrToValue(Type::kFreq, "5Hz", &success) == 10.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kFreq, "30000Hz", &success) == 20000.0f);
	REQUIRE(success == true);

	// 無効な文字列
	success = true;
	REQUIRE(StrToValue(Type::kFreq, "10invalidHz", &success) == 10.0f);
	REQUIRE(success == false);
	success = true;
	REQUIRE(StrToValue(Type::kFreq, "10invalidkHz", &success) == 10.0f);
	REQUIRE(success == false);
}

TEST_CASE("Parse pitch type parameters", "[AudioEffect][Param][Pitch]")
{
	bool success = false;

	// 浮動小数点形式(量子化なし、正の値で返される)
	success = false;
	REQUIRE(StrToValue(Type::kPitch, "12.0", &success) == Approx(60.0f)); // 12.0 + 48.0
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kPitch, "-6.5", &success) == Approx(41.5f)); // -6.5 + 48.0
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kPitch, "0.0", &success) == Approx(48.0f)); // 0.0 + 48.0
	REQUIRE(success == true);

	// 整数形式(量子化あり、負の値で返される)
	success = false;
	REQUIRE(StrToValue(Type::kPitch, "12", &success) == Approx(-60.0f)); // -(12 + 48.0)
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kPitch, "-6", &success) == Approx(-42.0f)); // -(-6 + 48.0)
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kPitch, "0", &success) == Approx(-48.0f)); // -(0 + 48.0)
	REQUIRE(success == true);

	// 境界値
	success = false;
	REQUIRE(StrToValue(Type::kPitch, "48.0", &success) == Approx(96.0f)); // 48.0 + 48.0
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kPitch, "-48.0", &success) == Approx(0.0f)); // -48.0 + 48.0
	REQUIRE(success == true);

	// 範囲外の値は丸める
	success = false;
	REQUIRE(StrToValue(Type::kPitch, "50.0", &success) == Approx(96.0f)); // 48.0 + 48.0
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kPitch, "-50.0", &success) == Approx(0.0f)); // -48.0 + 48.0
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kPitch, "50", &success) == Approx(-96.0f)); // -(48 + 48.0)
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kPitch, "-50", &success) == Approx(0.0f)); // -(-48 + 48.0)
	REQUIRE(success == true);
}

TEST_CASE("Parse int type parameters", "[AudioEffect][Param][Int]")
{
	bool success = false;

	// 整数値
	success = false;
	REQUIRE(StrToValue(Type::kInt, "10", &success) == 10.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kInt, "-5", &success) == -5.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kInt, "0", &success) == 0.0f);
	REQUIRE(success == true);
}

TEST_CASE("Parse float type parameters", "[AudioEffect][Param][Float]")
{
	bool success = false;

	// 浮動小数点値
	success = false;
	REQUIRE(StrToValue(Type::kFloat, "2.5", &success) == 2.5f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kFloat, "-10", &success) == -10.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kFloat, "0.707", &success) == Approx(0.707f));
	REQUIRE(success == true);
}

TEST_CASE("Parse dB type parameters", "[AudioEffect][Param][DB]")
{
	bool success = false;

	// dB形式
	success = false;
	REQUIRE(StrToValue(Type::kDB, "-8.0dB", &success) == -8.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kDB, "0dB", &success) == 0.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kDB, "3.5dB", &success) == 3.5f);
	REQUIRE(success == true);

	// dBサフィックスがない場合は0.0fを返す
	success = false;
	REQUIRE(StrToValue(Type::kDB, "-8.0", &success) == 0.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kDB, "3.5", &success) == 0.0f);
	REQUIRE(success == true);
}

TEST_CASE("Parse unspecified type parameters", "[AudioEffect][Param][Unspecified]")
{
	bool success = false;

	// kUnspecifiedは常に0.0fを返す
	success = false;
	REQUIRE(StrToValue(Type::kUnspecified, "any value", &success) == 0.0f);
	REQUIRE(success == true);
	success = false;
	REQUIRE(StrToValue(Type::kUnspecified, "123", &success) == 0.0f);
	REQUIRE(success == true);
}

TEST_CASE("Parse parameter value sets (Off/OnMin/OnMax)", "[AudioEffect][Param][ValueSet]")
{
	bool success = false;

	// 単一値(OnMin/OnMaxがOffを継承)
	{
		success = false;
		const ValueSet vs = StrToValueSet(Type::kRate, "50%", &success);
		REQUIRE(success == true);
		REQUIRE(vs.off == 0.5f);
		REQUIRE(vs.onMin == 0.5f);
		REQUIRE(vs.onMax == 0.5f);
	}

	// ">"区切り(OnMaxがOnMinを継承)
	{
		success = false;
		const ValueSet vs = StrToValueSet(Type::kRate, "20%>100%", &success);
		REQUIRE(success == true);
		REQUIRE(vs.off == 0.2f);
		REQUIRE(vs.onMin == 1.0f);
		REQUIRE(vs.onMax == 1.0f);
	}

	// "-"区切り(OffがOnMinを継承)
	{
		success = false;
		const ValueSet vs = StrToValueSet(Type::kRate, "50%-100%", &success);
		REQUIRE(success == true);
		REQUIRE(vs.off == 0.5f);
		REQUIRE(vs.onMin == 0.5f);
		REQUIRE(vs.onMax == 1.0f);
	}

	// フル指定
	{
		success = false;
		const ValueSet vs = StrToValueSet(Type::kRate, "10%>50%-100%", &success);
		REQUIRE(success == true);
		REQUIRE(vs.off == 0.1f);
		REQUIRE(vs.onMin == 0.5f);
		REQUIRE(vs.onMax == 1.0f);
	}

	// pitch型で負の値を含む"-"区切り
	{
		success = false;
		const ValueSet vs = StrToValueSet(Type::kPitch, "-24.0-24.0", &success);
		REQUIRE(success == true);
		REQUIRE(vs.off == Approx(24.0f));
		REQUIRE(vs.onMin == Approx(24.0f));
		REQUIRE(vs.onMax == Approx(72.0f));
	}

	// pitch型で負の値を含むフル指定
	{
		success = false;
		const ValueSet vs = StrToValueSet(Type::kPitch, "-12.0>-6.0-6.0", &success);
		REQUIRE(success == true);
		REQUIRE(vs.off == Approx(36.0f));
		REQUIRE(vs.onMin == Approx(42.0f));
		REQUIRE(vs.onMax == Approx(54.0f));
	}

	// dB型で負の値を含むフル指定
	{
		success = false;
		const ValueSet vs = StrToValueSet(Type::kDB, "-8.0dB>-3.0dB--1.0dB", &success);
		REQUIRE(success == true);
		REQUIRE(vs.off == -8.0f);
		REQUIRE(vs.onMin == -3.0f);
		REQUIRE(vs.onMax == -1.0f);
	}

	// pitch型で負の整数値(量子化あり)
	{
		success = false;
		const ValueSet vs = StrToValueSet(Type::kPitch, "-12>-6-6", &success);
		REQUIRE(success == true);
		REQUIRE(vs.off == Approx(-36.0f));
		REQUIRE(vs.onMin == Approx(-42.0f));
		REQUIRE(vs.onMax == Approx(-54.0f));
	}

	// pitch型でOnMin/OnMax同士は量子化の有無が統一される
	{
		// OnMinのみ小数
		success = false;
		const ValueSet vs1 = StrToValueSet(Type::kPitch, "0>6.0-12", &success);
		REQUIRE(success == true);
		REQUIRE(vs1.off == Approx(-48.0f)); // offは影響を受けない
		REQUIRE(vs1.onMin == Approx(54.0f));
		REQUIRE(vs1.onMax == Approx(60.0f)); // 12.0(量子化あり)ではなく60.0(量子化なし)

		// OnMaxのみ小数
		success = false;
		const ValueSet vs2 = StrToValueSet(Type::kPitch, "0>6-12.0", &success);
		REQUIRE(success == true);
		REQUIRE(vs2.off == Approx(-48.0f)); // offは影響を受けない
		REQUIRE(vs2.onMin == Approx(54.0f)); // 6.0(量子化あり)ではなく54.0(量子化なし)
		REQUIRE(vs2.onMax == Approx(60.0f));
	}

	// length型でOnMin/OnMaxの符号が異なる場合は失敗
	{
		bool success = true;

		// テンポ同期と非テンポ同期の混在は失敗
		const ValueSet vs1 = StrToValueSet(Type::kLength, "1/4-100ms", &success);
		REQUIRE(success == false);
		REQUIRE(vs1.off == 0.0f);
		REQUIRE(vs1.onMin == 0.0f);
		REQUIRE(vs1.onMax == 0.0f);

		// 両方が非テンポ同期なら成功
		success = false;
		const ValueSet vs2 = StrToValueSet(Type::kLength, "100ms-200ms", &success);
		REQUIRE(success == true);
		REQUIRE(vs2.off == Approx(-0.1f));
		REQUIRE(vs2.onMin == Approx(-0.1f));
		REQUIRE(vs2.onMax == Approx(-0.2f));

		// 両方がテンポ同期なら成功
		success = false;
		const ValueSet vs3 = StrToValueSet(Type::kLength, "1/4-1/2", &success);
		REQUIRE(success == true);
		REQUIRE(vs3.off == 0.25f);
		REQUIRE(vs3.onMin == 0.25f);
		REQUIRE(vs3.onMax == 0.5f);
	}
}

TEST_CASE("Parse non-finite parameter values", "[AudioEffect][Param][NonFinite]")
{
	// nan/infはどの型でも無効な文字列として扱う
	const std::pair<Type, const char*> cases[] = {
		{ Type::kLength, "nan" },
		{ Type::kLength, "inf" },
		{ Type::kLength, "infms" },
		{ Type::kWaveLength, "inf" },
		{ Type::kSample, "infsamples" },
		{ Type::kRate, "nan" },
		{ Type::kRate, "inf" },
		{ Type::kRate, "inf%" },
		{ Type::kFreq, "nanHz" },
		{ Type::kFreq, "infHz" },
		{ Type::kFreq, "infkHz" },
		{ Type::kPitch, "nan" },
		{ Type::kPitch, "inf" },
		{ Type::kFloat, "nan" },
		{ Type::kFloat, "-inf" },
		{ Type::kDB, "infdB" },
	};
	for (const auto& [type, str] : cases)
	{
		bool success = true;
		const ValueSet vs = StrToValueSet(type, str, &success);
		INFO(str);
		REQUIRE(success == false);
		REQUIRE(vs.off == 0.0f);
		REQUIRE(vs.onMin == 0.0f);
		REQUIRE(vs.onMax == 0.0f);
	}

	// Off>OnMin-OnMaxのいずれかがnan/infの場合も失敗
	bool success = true;
	const ValueSet vs = StrToValueSet(Type::kRate, "0%>inf", &success);
	REQUIRE(success == false);
	REQUIRE(vs.off == 0.0f);
	REQUIRE(vs.onMin == 0.0f);
	REQUIRE(vs.onMax == 0.0f);
}

TEST_CASE("Convert parameter string dictionary", "[AudioEffect][Param][Dict]")
{
	// ratioは小数を保持する
	{
		const ParamValueSetDict dict = StrDictToParamValueSetDict({ { "ratio", "2.5>7.5" } });
		REQUIRE(dict.contains(ParamID::kRatio));
		REQUIRE(dict.at(ParamID::kRatio).off == 2.5f);
		REQUIRE(dict.at(ParamID::kRatio).onMin == 7.5f);
		REQUIRE(dict.at(ParamID::kRatio).onMax == 7.5f);
	}

	// bandwidthも変換される
	{
		const ParamValueSetDict dict = StrDictToParamValueSetDict({ { "bandwidth", "1.414" } });
		REQUIRE(dict.contains(ParamID::kBandwidth));
		REQUIRE(dict.at(ParamID::kBandwidth).off == 1.414f);
	}
}

TEST_CASE("ParamSpec default values match Params definitions", "[AudioEffect][Param][Spec]")
{
	const auto check = [](std::string_view effectType, const std::unordered_map<ParamID, Param*>& dict)
	{
		const auto* pSpecs = GetParamSpecs(effectType);
		REQUIRE(pSpecs != nullptr);
		REQUIRE(pSpecs->size() == dict.size());
		for (const auto& spec : *pSpecs)
		{
			INFO(effectType << "." << spec.name);
			REQUIRE(dict.contains(spec.id));
			const Param& param = *dict.at(spec.id);
			REQUIRE(param.type == spec.type);
			const ValueSet expected = StrToValueSet(spec.type, std::string{ spec.defaultValue });
			REQUIRE(param.valueSet.off == expected.off);
			REQUIRE(param.valueSet.onMin == expected.onMin);
			REQUIRE(param.valueSet.onMax == expected.onMax);
		}
	};

	RetriggerParams retrigger;
	check("retrigger", retrigger.dict);
	GateParams gate;
	check("gate", gate.dict);
	FlangerParams flanger;
	check("flanger", flanger.dict);
	PitchShiftParams pitchShift;
	check("pitch_shift", pitchShift.dict);
	BitcrusherParams bitcrusher;
	check("bitcrusher", bitcrusher.dict);
	PhaserParams phaser;
	check("phaser", phaser.dict);
	WobbleParams wobble;
	check("wobble", wobble.dict);
	TapestopParams tapestop;
	check("tapestop", tapestop.dict);
	EchoParams echo;
	check("echo", echo.dict);
	SidechainParams sidechain;
	check("sidechain", sidechain.dict);
	PeakingFilterParams peakingFilter;
	check("peaking_filter", peakingFilter.dict);
	HighPassFilterParams highPassFilter;
	check("high_pass_filter", highPassFilter.dict);
	LowPassFilterParams lowPassFilter;
	check("low_pass_filter", lowPassFilter.dict);
}

TEST_CASE("Peaking filter output is finite near Nyquist frequency", "[AudioEffect][BiquadFilter]")
{
	for (const float sampleRate : { 22050.0f, 32000.0f, 44100.0f, 48000.0f })
	{
		INFO(sampleRate);
		ksmaudio::AudioEffect::detail::BiquadFilter<float> filter;
		filter.setPeakingFilter(20000.0f, 10.0f, 17.0f, sampleRate);
		for (int i = 0; i < 100; ++i)
		{
			const float output = filter.process(0.1f * std::sin(i * 0.3f));
			REQUIRE(std::isfinite(output));
		}
	}
}

TEST_CASE("Validate length type parameters with overflowing integers", "[AudioEffect][Param][Validator]")
{
	using ksmaudio::AudioEffect::Type;
	using ksmaudio::AudioEffect::ValidateParamValue;

	REQUIRE(ValidateParamValue(Type::kLength, "1/4").isValid);
	REQUIRE(ValidateParamValue(Type::kLength, "1/2147483647").isValid);

	// intの範囲を超える分母は、longがintより大きい環境でも無効
	REQUIRE_FALSE(ValidateParamValue(Type::kLength, "1/2147483648").isValid);
	REQUIRE_FALSE(ValidateParamValue(Type::kLength, "1/4294967297").isValid);
	REQUIRE_FALSE(ValidateParamValue(Type::kLength, "1/99999999999999999999").isValid);
}

TEST_CASE("Validate sample/rate/freq/int type parameters with overflowing integers", "[AudioEffect][Param][Validator]")
{
	using ksmaudio::AudioEffect::Type;
	using ksmaudio::AudioEffect::ValidateParamValue;

	REQUIRE(ValidateParamValue(Type::kInt, "2147483647").isValid);

	REQUIRE_FALSE(ValidateParamValue(Type::kSample, "2147483648samples").isValid);
	REQUIRE_FALSE(ValidateParamValue(Type::kRate, "2147483648%").isValid);
	REQUIRE_FALSE(ValidateParamValue(Type::kFreq, "2147483648Hz").isValid);
	REQUIRE_FALSE(ValidateParamValue(Type::kInt, "2147483648").isValid);
}

TEST_CASE("Validate float values out of the float range", "[AudioEffect][Param][Validator]")
{
	using ksmaudio::AudioEffect::Type;
	using ksmaudio::AudioEffect::ValidateParamValue;

	const std::string big(50, '9');

	REQUIRE(ValidateParamValue(Type::kFloat, "1.5").isValid);

	REQUIRE_FALSE(ValidateParamValue(Type::kLength, big + "ms").isValid);
	REQUIRE_FALSE(ValidateParamValue(Type::kLength, big).isValid);
	REQUIRE_FALSE(ValidateParamValue(Type::kFloat, big).isValid);
	REQUIRE_FALSE(ValidateParamValue(Type::kDB, big + "dB").isValid);
}
