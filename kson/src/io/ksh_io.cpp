#include "kson/io/ksh_io.hpp"
#include "kson/encoding/encoding.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <optional>
#include <charconv>
#include <cmath>

namespace
{
	using namespace kson;

	constexpr char kOptionSeparator = '=';
	constexpr char kBlockSeparator = '|';
	constexpr std::string_view kMeasureSeparator = "--";
	constexpr char kAudioEffectStrSeparator = ';';

	enum BlockIdx : std::size_t
	{
		kBlockIdxBT = 0,
		kBlockIdxFX,
		kBlockIdxLaser,
	};

	// Maximum value of zoom
	constexpr double kZoomAbsMaxLegacy = 300.0;  // ver <  1.67
	constexpr double kZoomAbsMax = 65535.0;      // ver >= 1.67

	// Maximum number of characters of the zoom value
	constexpr std::size_t kZoomMaxCharLegacy = 4;       // ver <  1.67
	constexpr std::size_t kZoomMaxChar = std::string::npos; // ver >= 1.67

	// Maximum value of center_split / manual tilt
	constexpr double kCenterSplitAbsMax = 65535.0;
	constexpr double kManualTiltAbsMax = 1000.0;

	constexpr std::int32_t kRotationFlagTilt = 1 << 0;
	constexpr std::int32_t kRotationFlagSpin = 1 << 1;

	constexpr std::int32_t kAudioEffectParamUnspecified = -99999;
	const std::string kAudioEffectParamUnspecifiedStr = std::to_string(kAudioEffectParamUnspecified);

	template <typename T>
	T ParseNumeric(std::string_view str, T defaultValue = T{ 0 })
	{
		T result{ defaultValue };
		std::from_chars_result r;
		if constexpr (std::is_integral_v<T>)
		{
			r = std::from_chars(str.data(), str.data() + str.size(), result, 10);
		}
		else
		{
			r = std::from_chars(str.data(), str.data() + str.size(), result, std::chars_format::fixed);
		}

		if (r.ec == std::errc{})
		{
			return static_cast<T>(result);
		}

		return defaultValue;
	}

	template <typename T, typename U>
	T ParseNumeric(const std::basic_string<U>& str, T defaultValue = T{ 0 })
	{
		return ParseNumeric<T>(std::basic_string_view<U>(str), defaultValue);
	}

	std::string ToUTF8(std::string_view str, bool isUTF8)
	{
		if (isUTF8)
		{
			return std::string(str.cbegin(), str.cend());
		}
		else
		{
			return Encoding::ShiftJISToUTF8(str);
		}
	}

	bool IsChartLine(std::string_view line)
	{
		return line.find(kBlockSeparator) != std::string_view::npos;
	}

	bool IsOptionLine(std::string_view line)
	{
		return line.find(kOptionSeparator) != std::string_view::npos;
	}

	bool IsBarLine(std::string_view line)
	{
		return line == kMeasureSeparator;
	}

	bool IsCommentLine(std::string_view line)
	{
		return line.length() >= 2 && line[0] == '/' && line[1] == '/';
	}

	std::pair<std::string, std::string> SplitOptionLine(std::string_view optionLine, bool isUTF8)
	{
		const std::string optionLineUTF8 = ToUTF8(optionLine, isUTF8);
		const std::size_t equalIdx = optionLineUTF8.find_first_of(kOptionSeparator);

		// Option line must have "="
		assert(equalIdx != std::string_view::npos);
		if (equalIdx == std::string_view::npos)
		{
			return std::pair<std::string, std::string>();
		}

		return {
			optionLineUTF8.substr(0, equalIdx),
			optionLineUTF8.substr(equalIdx + 1)
		};
	}

	std::tuple<std::string, std::int32_t, std::int32_t> SplitAudioEffectStr(std::string_view optionLine)
	{
		using Tuple = std::tuple<std::string, std::int32_t, std::int32_t>;

		const std::size_t semicolonIdx1 = optionLine.find_first_of(kAudioEffectStrSeparator);
		if (semicolonIdx1 == std::string_view::npos)
		{
			return Tuple{ optionLine, kAudioEffectParamUnspecified, kAudioEffectParamUnspecified };
		}

		const std::size_t semicolonIdx2 = optionLine.substr(semicolonIdx1 + 1).find_first_of(kAudioEffectStrSeparator);
		const std::int32_t param1 = ParseNumeric<std::int32_t>(optionLine.substr(semicolonIdx1 + 1));
		if (semicolonIdx2 == std::string_view::npos)
		{
			return Tuple{ optionLine.substr(0, semicolonIdx1), param1, kAudioEffectParamUnspecified };
		}

		const std::int32_t param2 = ParseNumeric<std::int32_t>(optionLine.substr(semicolonIdx1 + semicolonIdx2 + 2));

		return Tuple{ optionLine.substr(0, semicolonIdx1), param1, param2 };
	}

	template <std::size_t N>
	std::array<std::string, N> Split(std::string_view str, char separator)
	{
		std::array<std::string, N> arr;

		std::size_t cursor = 0;
		for (std::size_t i = 0; i < N; ++i)
		{
			std::string_view s = str.substr(cursor);
			const std::size_t semicolonIdx = s.find_first_of(separator);
			if (semicolonIdx == std::string_view::npos)
			{
				arr[i] = s;
				break;
			}
			arr[i] = s.substr(0, semicolonIdx);
			cursor += semicolonIdx + 1;
		}

		return arr;
	}

	std::string_view KSHLegacyFXCharToKSHAudioEffectStr(char c)
	{
		switch (c)
		{
		case 'S': return "Retrigger;8";
		case 'V': return "Retrigger;12";
		case 'T': return "Retrigger;16";
		case 'W': return "Retrigger;24";
		case 'U': return "Retrigger;32";
		case 'G': return "Gate;4";
		case 'H': return "Gate;8";
		case 'K': return "Gate;12";
		case 'I': return "Gate;16";
		case 'L': return "Gate;24";
		case 'J': return "Gate;32";
		case 'F': return "Flanger";
		case 'P': return "PitchShift;12";
		case 'B': return "BitCrusher;5";
		case 'Q': return "Phaser";
		case 'X': return "Wobble;12";
		case 'A': return "TapeStop;17";
		case 'D': return "SideChain";
		default:  return "";
		}
	}

	const std::unordered_map<std::string_view, std::string_view> s_kshFXToKSONAudioEffectNameTable
	{
		{ "Retrigger", "retrigger" },
		{ "Gate", "gate" },
		{ "Flanger", "flanger" },
		{ "PitchShift", "pitch_shift" },
		{ "BitCrusher", "bitcrusher" },
		{ "Phaser", "phaser" },
		{ "Wobble", "wobble" },
		{ "TapeStop", "tapestop" },
		{ "Echo", "echo" },
		{ "SideChain", "sidechain" },
	};

	const std::unordered_map<std::string_view, std::string_view> s_kshFilterToKSONAudioEffectNameTable
	{
		{ "peak", "peaking_filter" },
		{ "hpf1", "high_pass_filter" },
		{ "lpf1", "low_pass_filter" },
		{ "bitc", "bitcrusher" },
		{ "fx", "fx" },
		{ "fx;bitc", "fx;bitcrusher" },
	};

	const std::unordered_map<std::string_view, AudioEffectType> s_audioEffectTypeTable
	{
		{ "Retrigger", AudioEffectType::Retrigger },
		{ "Gate", AudioEffectType::Gate },
		{ "Flanger", AudioEffectType::Flanger },
		{ "PitchShift", AudioEffectType::PitchShift },
		{ "BitCrusher", AudioEffectType::Bitcrusher },
		{ "Phaser", AudioEffectType::Phaser },
		{ "Wobble", AudioEffectType::Wobble },
		{ "TapeStop", AudioEffectType::Tapestop },
		{ "Echo", AudioEffectType::Echo },
		{ "SideChain", AudioEffectType::Sidechain },
		{ "SwitchAudio", AudioEffectType::SwitchAudio },

		// Note: These filters are not yet supported in the KSH format, but are added to convert future charts.
		{ "PeakingFilter", AudioEffectType::PeakingFilter },
		{ "HighPassFilter", AudioEffectType::HighPassFilter },
		{ "LowPassFilter", AudioEffectType::LowPassFilter },
	};

	const std::unordered_map<std::string_view, std::string_view> s_audioEffectParamNameTable
	{
		{ "updatePeriod", "update_period" },
		{ "waveLength", "wave_length" },
		{ "rate", "rate" },
		{ "updateTrigger", "update_trigger" },
		{ "mix", "mix" },
		{ "period", "period" },
		{ "delay", "delay" },
		{ "depth", "depth" },
		{ "feedback", "feedback" },
		{ "stereoWidth", "stereo_width" },
		{ "volume", "vol" },
		{ "pitch", "pitch" },
		{ "chunkSize", "chunk_size" },
		{ "overWrap", "overlap" },
		{ "reduction", "reduction" },
		{ "stage", "stage" },
		{ "loFreq", "lo_freq" },
		{ "hiFreq", "hi_freq" },
		{ "Q", "q" },
		{ "speed", "speed" },
		{ "trigger", "trigger" },
		{ "feedbackLevel", "feedback_level" },
		{ "holdTime", "hold_time" },
		{ "attackTime", "attack_time" },
		{ "releaseTime", "release_time" },
		{ "ratio", "ratio" },
		{ "fileName", "filename" },
		{ "v", "v" },
		{ "freq", "freq" },
		{ "freqMax", "freq_max" },
		{ "gain", "gain" },
	};

	static constexpr std::int32_t kLaserXMax = 100;

	std::int32_t CharToLaserX(char c)
	{
		if (c >= '0' && c <= '9')
		{
			return (c - '0') * kLaserXMax / 50;
		}
		else if (c >= 'A' && c <= 'Z')
		{
			return (c - 'A' + 10) * kLaserXMax / 50;
		}
		else if (c >= 'a' && c <= 'o')
		{
			return (c - 'a' + 36) * kLaserXMax / 50;
		}
		else
		{
			return -1;
		}
	}

	bool IsTiltValueManual(std::string_view tiltValueStr)
	{
		return !tiltValueStr.empty() && ((tiltValueStr[0] >= '0' && tiltValueStr[0] <= '9') || tiltValueStr[0] == '-');
	}

	TimeSig ParseTimeSig(std::string_view str)
	{
		std::size_t slashIdx = str.find('/');

		// TimeSig must have "/"
		assert(slashIdx != std::string::npos);

		return TimeSig{
			ParseNumeric<std::int64_t>(str.substr(0, slashIdx)),
			ParseNumeric<std::int64_t>(str.substr(slashIdx + 1))
		};
	}

	bool EliminateUTF8BOM(std::istream& stream)
	{
		bool isUTF8;
		std::string firstLine;
		std::getline(stream, firstLine, '\n');
		if (firstLine.length() >= 3 &&
			firstLine[0] == '\xEF' &&
			firstLine[1] == '\xBB' &&
			firstLine[2] == '\xBF')
		{
			isUTF8 = true;
			stream.seekg(3, std::ios_base::beg);
		}
		else
		{
			isUTF8 = false;
			stream.seekg(0, std::ios_base::beg);
		}
		return isUTF8;
	}

	bool InsertBPMChange(ByPulse<double>& bpmChanges, Pulse time, std::string_view value)
	{
		if (value.find('-') != std::string_view::npos)
		{
			return false;
		}

		bpmChanges.insert_or_assign(time, ParseNumeric<double>(value));
		return true;
	}

	void InsertFiltertype(ChartData& chartData, Pulse time, const std::string& value)
	{
		if (s_kshFilterToKSONAudioEffectNameTable.contains(value))
		{
			std::string name(s_kshFilterToKSONAudioEffectNameTable.at(value));
			if (name == "fx" && !chartData.audio.audioEffect.laser.def.contains(name))
			{
				if (chartData.audio.bgm.legacy.filenameF.empty())
				{
					name.clear();
				}
				else
				{
					chartData.audio.audioEffect.laser.def.emplace("fx", AudioEffectDef{
						.type = AudioEffectType::SwitchAudio,
						.v = {
							{ "filename", chartData.audio.bgm.legacy.filenameF },
						},
					});
				}
			}
			else if (name == "fx;bitcrusher" && !chartData.audio.audioEffect.laser.def.contains(name))
			{
				chartData.audio.audioEffect.laser.def.emplace("fx;bitcrusher", AudioEffectDef{
					.type = AudioEffectType::Bitcrusher,
				});
			}
			if (!name.empty())
			{
				chartData.audio.audioEffect.laser.pulseEvent[name].insert(time);
			}
		}
		else
		{
			chartData.audio.audioEffect.laser.pulseEvent[value].insert(time);
		}
	}

	// TODO: refactor
	class AbstractPreparedLongNote
	{
	protected:
		bool m_prepared = false;

		Pulse m_time = 0;

		RelPulse m_length = 0;

		ChartData* m_pTargetChartData = nullptr; // TODO: this is troublesome

		std::size_t m_targetLaneIdx = 0;

	public:
		AbstractPreparedLongNote() = default;

		AbstractPreparedLongNote(ChartData* pTargetChartData, std::size_t targetLaneIdx)
			: m_pTargetChartData(pTargetChartData)
			, m_targetLaneIdx(targetLaneIdx)
		{
		}

		virtual ~AbstractPreparedLongNote() = 0;

		void prepare(Pulse time)
		{
			if (!m_prepared)
			{
				m_prepared = true;
				m_time = time;
				m_length = 0;
			}
		}

		bool prepared() const
		{
			return m_prepared;
		}

		void extendLength(RelPulse relPulse)
		{
			m_length += relPulse;
		}

		virtual void clear()
		{
			m_prepared = false;
			m_time = 0;
			m_length = 0;
		}
	};

	AbstractPreparedLongNote::~AbstractPreparedLongNote() = default;

	class PreparedLongBTNote : public AbstractPreparedLongNote
	{
	public:
		PreparedLongBTNote() = default;

		PreparedLongBTNote(ChartData* pTargetChartData, std::size_t targetLaneIdx)
			: AbstractPreparedLongNote(pTargetChartData, targetLaneIdx)
		{
		}

		virtual ~PreparedLongBTNote() = default;

		void publishLongBTNote()
		{
			if (!m_prepared)
			{
				return;
			}

			m_pTargetChartData->note.bt[m_targetLaneIdx].emplace(
				m_time,
				m_length);

			clear();
		}
	};

	class PreparedLongFXNote : public AbstractPreparedLongNote // TODO: remove this bad inheritance
	{
	private:
		std::optional<std::string> m_audioEffectStr;
		std::optional<std::string> m_audioEffectParamStr;
		bool m_isLegacyChar = false;

	public:
		PreparedLongFXNote() = default; // TODO: this is troublesome

		PreparedLongFXNote(ChartData* pTargetChartData, std::size_t targetLaneIdx)
			: AbstractPreparedLongNote(pTargetChartData, targetLaneIdx)
		{
		}

		virtual ~PreparedLongFXNote() = default;

		void prepare(Pulse time)
		{
			if (m_isLegacyChar)
			{
				prepare(time, "", "", false); // If the long note starts with a legacy character (e.g., "F" = Flanger), insert no audio effect when it changes to "1"
				return;
			}

			AbstractPreparedLongNote::prepare(time);
		}

		// audioEffectStr: FX audio effect string ("fx-l=" or "fx-r=" in .ksh, e.g., "Retrigger;16", "Echo;8;70")
		// audioEffectParamStr: Legacy FX audio effect parameters ("fx-l_param1=" or "fx-r_param1=" in .ksh)
		void prepare(Pulse time, std::string_view audioEffectStr, std::string_view audioEffectParamStr, bool isLegacyChar)
		{
			if ((audioEffectStr != m_audioEffectStr || audioEffectParamStr != m_audioEffectParamStr) && (!audioEffectStr.empty() || m_prepared))
			{
				publishLongFXAudioEffectEvent(time, audioEffectStr, audioEffectParamStr);
			}
			m_audioEffectStr = audioEffectStr;
			m_audioEffectParamStr = audioEffectParamStr;
			m_isLegacyChar = isLegacyChar;

			AbstractPreparedLongNote::prepare(time);
		}

		void publishLongFXNote()
		{
			if (!m_prepared)
			{
				return;
			}

			// Publish prepared long FX note
			m_pTargetChartData->note.fx[m_targetLaneIdx].emplace(m_time, m_length);

			clear();
		}

		void publishLongFXAudioEffectEvent(Pulse time, std::string_view audioEffectStr, std::string_view audioEffectParamStr)
		{
			auto [audioEffectName, audioEffectParamValue1, audioEffectParamValue2] = SplitAudioEffectStr(audioEffectStr);
			if (!audioEffectParamStr.empty())
			{
				// Apply legacy FX audio effect parameters
				// Note: Legacy parameters do not support audioEffectParamValue2 (for Echo), so only audioEffectParamValue1 is sufficient.
				audioEffectParamValue1 = ParseNumeric<std::int32_t>(audioEffectParamStr);
			}
			if (s_kshFXToKSONAudioEffectNameTable.contains(audioEffectName))
			{
				// Convert the name of preset audio effects
				audioEffectName = s_kshFXToKSONAudioEffectNameTable.at(audioEffectName);
			}
			m_pTargetChartData->audio.audioEffect.fx.longEvent[audioEffectName][m_targetLaneIdx].emplace(time, AudioEffectParams{
				// Store the value of the parameters in temporary keys
				// (Since the conversion requires determining the type of audio effect, it is processed
				//  after reading the "#define_fx"/"#define_filter" lines.)
				{ "_param1", std::to_string(audioEffectParamValue1) },
				{ "_param2", std::to_string(audioEffectParamValue2) },
			});
		}

		virtual void clear() override
		{
			m_audioEffectStr = std::nullopt;
			m_audioEffectParamStr = std::nullopt;
			m_isLegacyChar = false;
			AbstractPreparedLongNote::clear();
		}
	};

	RelPulse KSHLengthToRelPulse(std::string_view str)
	{
		return ParseNumeric<RelPulse>(str) * kResolution4 / 192;
	}

	std::tuple<RelPulse, std::int32_t, std::int32_t, std::int32_t> SplitSwingParams(std::string_view paramStr)
	{
		std::array<std::string, 4> params{
			"192", "250", "3", "2"
		};

		const std::string paramStrClone(paramStr);
		std::stringstream ss(paramStrClone);
		std::string s;
		int i = 0;
		while (i < 4 && std::getline(ss, s, ';'))
		{
			params[i] = s;
			++i;
		}

		return std::make_tuple(
			KSHLengthToRelPulse(params[0]),
			ParseNumeric<std::int32_t>(params[1]),
			ParseNumeric<std::int32_t>(params[2]),
			ParseNumeric<std::int32_t>(params[3]));
	}

	constexpr double kKSHToKSONCamScale = 1.0 / 100.0;
	constexpr double kKSHToKSONTiltScale = 1.0 / 100.0;
	constexpr double kKSHToKSONSwingScale = 1.0 / 60.0;

	struct PreparedLaneSpin
	{
		enum class Type
		{
			kNoSpin,
			kNormal,
			kHalf,
			kSwing,
		};
		Type type = Type::kNoSpin;

		enum class Direction
		{
			kUnspecified,
			kLeft,
			kRight,
		};
		Direction direction = Direction::kUnspecified;

		RelPulse duration = 0;

		std::int32_t swingAmplitude = 0;

		std::int32_t swingRepeat = 0;

		std::int32_t swingDecayOrder = 0;

		static PreparedLaneSpin FromKSHSpinStr(std::string_view strFromKsh) // From .ksh spin string (example: "@(192")
		{
			// A .ksh spin string should have at least 3 chars
			if (strFromKsh.length() < 3)
			{
				return {
					.type = Type::kNoSpin,
					.direction = Direction::kUnspecified,
					.duration = 0,
				};
			}

			// Specify the spin type
			Type type;
			Direction direction;
			if (strFromKsh[0] == '@')
			{
				switch (strFromKsh[1])
				{
				case '(':
					type = Type::kNormal;
					direction = Direction::kLeft;
					break;

				case ')':
					type = Type::kNormal;
					direction = Direction::kRight;
					break;

				case '<':
					type = Type::kHalf;
					direction = Direction::kLeft;
					break;

				case '>':
					type = Type::kHalf;
					direction = Direction::kRight;
					break;

				default:
					type = Type::kNoSpin;
					direction = Direction::kUnspecified;
					break;
				}
			}
			else if (strFromKsh[0] == 'S')
			{
				switch (strFromKsh[1])
				{
				case '<':
					type = Type::kSwing;
					direction = Direction::kLeft;
					break;

				case '>':
					type = Type::kSwing;
					direction = Direction::kRight;
					break;

				default:
					type = Type::kNoSpin;
					direction = Direction::kUnspecified;
					break;
				}
			}
			else
			{
				type = Type::kNoSpin;
				direction = Direction::kUnspecified;
			}

			// Specify the spin length
			RelPulse duration;
			std::int32_t swingAmplitude = 0, swingRepeat = 0, swingDecayOrder = 0;
			if (type == Type::kNoSpin || direction == Direction::kUnspecified)
			{
				duration = 0;
			}
			else if (type == Type::kSwing)
			{
				std::tie(duration, swingAmplitude, swingRepeat, swingDecayOrder) = SplitSwingParams(strFromKsh.substr(2));
			}
			else
			{
				duration = KSHLengthToRelPulse(strFromKsh.substr(2));
			}

			return {
				.type = type,
				.direction = direction,
				.duration = duration,
				.swingAmplitude = swingAmplitude,
				.swingRepeat = swingRepeat,
				.swingDecayOrder = swingDecayOrder,
			};
		}

		bool isValid() const
		{
			return type != Type::kNoSpin && direction != Direction::kUnspecified;
		}
	};

	const std::unordered_map<PreparedLaneSpin::Type, std::string_view> s_kshSpinTypeToKsonCamPatternNameTable
	{
		{ PreparedLaneSpin::Type::kNoSpin, "" },
		{ PreparedLaneSpin::Type::kNormal, CamPatternKey::kSpin },
		{ PreparedLaneSpin::Type::kHalf, CamPatternKey::kHalfSpin },
		{ PreparedLaneSpin::Type::kSwing, CamPatternKey::kSwing },
	};

	const std::unordered_map<std::string_view, double> s_tiltTypeScaleTable
	{
		{ "normal", 1.0 },
		{ "bigger", 1.75 },
		{ "biggest", 2.5 },
		{ "keep_normal", 1.0 },
		{ "keep_bigger", 1.75 },
		{ "keep_biggest", 2.5 },
		{ "zero", 0.0 },
		{ "big", 1.75 },  // legacy
		{ "keep", 1.75 }, // legacy
	};

	const std::unordered_map<std::string_view, bool> s_tiltTypeKeepTable
	{
		{ "normal", false },
		{ "bigger", false },
		{ "biggest", false },
		{ "keep_normal", true },
		{ "keep_bigger", true },
		{ "keep_biggest", true },
		{ "zero", false },
		{ "big", false }, // legacy
		{ "keep", true }, // legacy
	};

	class PreparedGraphSection
	{
	protected:
		bool m_prepared = false;
		Pulse m_time = 0;
		ByRelPulse<GraphValue> m_values;
		ChartData* m_pTargetChartData = nullptr;

	public:
		PreparedGraphSection() = default;

		explicit PreparedGraphSection(ChartData* pTargetChartData)
			: m_pTargetChartData(pTargetChartData)
		{
		}

		virtual ~PreparedGraphSection() = default;

		void prepare(Pulse time)
		{
			if (!m_prepared)
			{
				m_prepared = true;
				m_time = time;
				m_values.clear();
			}
		}

		bool prepared() const
		{
			return m_prepared;
		}

		void addGraphPoint(Pulse time, double value)
		{
			const RelPulse relativeTime = time - m_time;

			if (relativeTime < 0)
			{
				return;
			}

			if (m_values.contains(relativeTime))
			{
				m_values.at(relativeTime).vf = value;
			}
			else
			{
				m_values.emplace(relativeTime, value);
			}
		}

		void publishManualTilt()
		{
			if (!m_prepared)
			{
				return;
			}

			m_pTargetChartData->camera.tilt.manual.emplace(
				m_time,
				m_values);

			clear();
		}

		virtual void clear()
		{
			m_prepared = false;
			m_time = 0;
			m_values.clear();
		}
	};

	class PreparedLaserSection : public PreparedGraphSection // TODO: remove this bad inheritance
	{
	private:
		std::size_t m_targetLaneIdx = 0;
		std::int32_t m_w = 1; // normal laser: w=1, 2x-widen laser: w=2
		ByRelPulse<PreparedLaneSpin> m_preparedLaneSpins;
		std::string m_keySound;

	public:
		PreparedLaserSection() = default;

		PreparedLaserSection(ChartData* pTargetChartData, std::size_t targetLaneIdx)
			: PreparedGraphSection(pTargetChartData)
			, m_targetLaneIdx(targetLaneIdx)
		{
		}

		virtual ~PreparedLaserSection() = default;

		void publishManualTilt() = delete;

		void prepare(Pulse) = delete;

		void prepare(Pulse time, std::int32_t w)
		{
			if (!m_prepared)
			{
				PreparedGraphSection::prepare(time);
				m_w = w;
			}
		}

		void publishLaserNote()
		{
			if (!m_prepared)
			{
				return;
			}

			assert(m_values.size() >= 2);

			// Convert a 32th or shorter laser segment to a laser slam
			const Pulse laserSlamThreshold = kResolution4 / 32;
			ByRelPulse<GraphValue> convertedGraphSection;
			for (auto itr = m_values.cbegin(); itr != m_values.cend(); ++itr)
			{
				const auto& [ry, value] = *itr;
				if (std::next(itr) != m_values.cend())
				{
					const auto& [nextRy, nextValue] = *std::next(itr);
					if (0 <= nextRy - ry && nextRy - ry <= laserSlamThreshold)
					{
						convertedGraphSection.emplace(ry, GraphValue{ value.v, nextValue.v });
						++itr;
						continue;
					}
				}

				convertedGraphSection.emplace(ry, value);
			}

			// Publish prepared laser section
			auto& targetLane = m_pTargetChartData->note.laser[m_targetLaneIdx];
			const auto [_, inserted] = targetLane.emplace(
				m_time,
				LaserSection{
					.v = convertedGraphSection,
					.w = m_w,
				});

			if (inserted)
			{
				// Publish prepared lane spin
				for (const auto& [relPulse, laneSpin] : m_preparedLaneSpins)
				{
					if (m_values.contains(relPulse) && laneSpin.isValid() && s_kshSpinTypeToKsonCamPatternNameTable.contains(laneSpin.type))
					{
						const std::string patternKey(s_kshSpinTypeToKsonCamPatternNameTable.at(laneSpin.type));

						CamPatternParams params;
						if (laneSpin.type == PreparedLaneSpin::Type::kSwing)
						{
							params = {
								.length = laneSpin.duration,
								.scale = static_cast<double>(laneSpin.swingAmplitude) * kKSHToKSONSwingScale,
								.repeat = laneSpin.swingRepeat,
								.decayOrder = laneSpin.swingDecayOrder,
							};
						}
						else
						{
							params = {
								.length = laneSpin.duration,
							};
						}

						if (!patternKey.empty())
						{
							m_pTargetChartData->camera.cam.pattern.laser.slamEvent[patternKey].emplace(m_time + relPulse, params);
						}
					}
				}
			}

			clear();
		}

		virtual void clear() override
		{
			PreparedGraphSection::clear();

			m_w = 1;
			m_preparedLaneSpins.clear();
		}

		void addLaneSpin(Pulse time, const PreparedLaneSpin& laneSpin)
		{
			m_preparedLaneSpins.emplace(time, laneSpin);
		}
	};

	template <class T, std::size_t N>
	std::array<T, N> MakePreparedLongNoteArray(ChartData* pTargetChartData)
	{
		std::array<T, N> arr;
		for (std::size_t i = 0; i < N; ++i)
		{
			arr[i] = T(pTargetChartData, i);
		}
		return arr;
	}

	struct PreparedLongNoteArray
	{
		std::array<PreparedLongBTNote, kNumBTLanes> bt;
		std::array<PreparedLongFXNote, kNumFXLanes> fx;
		std::array<PreparedLaserSection, kNumLaserLanes> laser;

		explicit PreparedLongNoteArray(ChartData* pTargetChartData)
			: bt(MakePreparedLongNoteArray<PreparedLongBTNote, kNumBTLanes>(pTargetChartData))
			, fx(MakePreparedLongNoteArray<PreparedLongFXNote, kNumFXLanes>(pTargetChartData))
			, laser(MakePreparedLongNoteArray<PreparedLaserSection, kNumLaserLanes>(pTargetChartData))
		{
		}
	};

	struct BufOptionLine
	{
		std::size_t lineIdx;
		std::string key;
		std::string value;
	};

	struct BufCommentLine
	{
		std::size_t lineIdx;
		std::string value;
	};

	struct BufUnknownLine
	{
		std::size_t lineIdx;
		std::string value;
	};

	struct BufKeySound
	{
		std::string name;
		std::int32_t vol;
	};

	std::string Pop(std::unordered_map<std::string, std::string>& meta, const std::string& key, std::string_view defaultValue = "")
	{
		if (meta.contains(key)) // Note: key is const string& instead of string_view because unordered_map<string, string>::contains() and at() do not support string_view as key
		{
			const std::string value = meta.at(key);
			meta.erase(key);
			return value;
		}
		else
		{
			return std::string(defaultValue);
		}
	}

	template <typename T>
	T PopInt(std::unordered_map<std::string, std::string>& meta, const std::string& key, T defaultValue = T{ 0 })
	{
		const std::string str = Pop(meta, key);
		if (str.empty())
		{
			return defaultValue;
		}
		return ParseNumeric<T>(str);
	}

	template <typename T>
	T PopInt(std::unordered_map<std::string, std::string>& meta, const std::string& key, T defaultValue, T minValue, T maxValue)
	{
		const T value = PopInt<T>(meta, key, defaultValue);
		return std::clamp(value, minValue, maxValue);
	}

	const std::unordered_map<std::string_view, std::int8_t> s_difficultyNameTable
	{
		{ "light", 0 },
		{ "challenge", 1 },
		{ "extended", 2 },
		{ "infinite", 3 },
	};

	template <typename ChartDataType>
	ChartDataType CreateChartDataFromMetaDataStream(std::istream& stream, bool* pIsUTF8)
		requires std::is_same_v<ChartDataType, kson::ChartData> || std::is_same_v<ChartDataType, kson::MetaChartData>
	{
		ChartDataType chartData;
		const bool isUTF8 = EliminateUTF8BOM(stream);
		if (pIsUTF8)
		{
			*pIsUTF8 = isUTF8;
		}

		// Read header lines and create meta data hash map
		[[maybe_unused]] bool barLineExists = false;
		std::unordered_map<std::string, std::string> metaDataHashMap;
		std::string line;
		while (std::getline(stream, line, '\n'))
		{
			// Eliminate CR
			if (!line.empty() && *line.crbegin() == '\r')
			{
				line.pop_back();
			}

			if (IsBarLine(line))
			{
				// Chart meta data is before the first bar line ("--")
				barLineExists = true;
				break;
			}

			// Comments
			if (IsCommentLine(line))
			{
				if constexpr (std::is_same_v<ChartDataType, ChartData>)
				{
					chartData.editor.comment.emplace(0, line.substr(2)); // 2 = strlen("//")
				}
				continue;
			}

			// Unexpected header lines
			if (!IsOptionLine(line))
			{
				if constexpr (std::is_same_v<ChartDataType, ChartData>)
				{
					chartData.compat.kshUnknown.line.emplace(0, line);
				}
				continue;
			}

			const auto [key, value] = SplitOptionLine(line, isUTF8);
			metaDataHashMap.insert_or_assign(key, value);
		}

		// .ksh files must have at least one bar line ("--")
		// TODO: Error handling
		assert(barLineExists);

		// Insert meta data to chartData
		{
			chartData.compat.kshVersion = Pop(metaDataHashMap, "ver", "");
			const std::int32_t kshVersionInt = ParseNumeric<std::int32_t>(chartData.compat.kshVersion, 170);

			chartData.meta.title = Pop(metaDataHashMap, "title");
			chartData.meta.artist = Pop(metaDataHashMap, "artist");
			chartData.meta.chartAuthor = Pop(metaDataHashMap, "effect");
			chartData.meta.jacketFilename = Pop(metaDataHashMap, "jacket");
			chartData.meta.jacketAuthor = Pop(metaDataHashMap, "illustrator");
			chartData.meta.iconFilename = Pop(metaDataHashMap, "icon");

			const std::string difficultyName = Pop(metaDataHashMap, "difficulty", "infinite");
			if (s_difficultyNameTable.contains(difficultyName))
			{
				chartData.meta.difficulty.idx = s_difficultyNameTable.at(difficultyName);
			}
			else
			{
				chartData.meta.difficulty.idx = 3;
			}

			chartData.meta.level = PopInt<std::int32_t>(metaDataHashMap, "level", 1, 1, 20);

			if constexpr (std::is_same_v<ChartDataType, ChartData>)
			{
				// Insert the first time signature change
				TimeSig firstTimeSig{ 4, 4 };
				if (metaDataHashMap.contains("beat")) [[unlikely]] // unlikely because "beat=" is usually after the first bar line
				{
					firstTimeSig = ParseTimeSig(metaDataHashMap.at("beat"));
					metaDataHashMap.erase("beat");
				}
				chartData.beat.timeSig.emplace(0, firstTimeSig);

				// Insert the first tempo change
				if (metaDataHashMap.contains("t")) [[likely]]
				{
					InsertBPMChange(chartData.beat.bpm, 0, metaDataHashMap.at("t"));
				}
			}
			chartData.meta.dispBPM = Pop(metaDataHashMap, "t", "");

			chartData.meta.stdBPM = ParseNumeric<double>(Pop(metaDataHashMap, "to", "0"), 0.0);

			const auto bgmFilenames = Split<4>(Pop(metaDataHashMap, "m"), ';');
			if constexpr (std::is_same_v<ChartDataType, ChartData>)
			{
				chartData.audio.bgm.filename = bgmFilenames[0];
				chartData.audio.bgm.legacy.filenameF = bgmFilenames[1];
				chartData.audio.bgm.legacy.filenameP = bgmFilenames[2];
				chartData.audio.bgm.legacy.filenameFP = bgmFilenames[3];
			}

			const std::int32_t volInt = PopInt<std::int32_t>(metaDataHashMap, "mvol", 50);
			chartData.audio.bgm.vol = volInt / 100.0;
			if (chartData.compat.kshVersion.empty())
			{
				// For historical reasons, the value is multiplied by 0.6 if the value of "ver" is not specified.
				chartData.audio.bgm.vol *= 0.6;
			}

			if constexpr (std::is_same_v<ChartDataType, ChartData>)
			{
				chartData.audio.bgm.offset = PopInt<std::int32_t>(metaDataHashMap, "o", 0);
			}
			chartData.audio.bgm.preview.offset = PopInt<std::int32_t>(metaDataHashMap, "po", 0);
			chartData.audio.bgm.preview.duration = PopInt<std::int32_t>(metaDataHashMap, "plength", 0);

			if constexpr (std::is_same_v<ChartDataType, ChartData>)
			{
				chartData.audio.keySound.laser.vol.emplace(0, static_cast<double>(PopInt<std::int32_t>(metaDataHashMap, "chokkakuvol", 50)) / 100);
				chartData.audio.keySound.laser.legacy.autoVol = PopInt<std::int32_t>(metaDataHashMap, "chokkakuautovol", 1) != 0;
				if (metaDataHashMap.contains("filtertype"))
				{
					InsertFiltertype(chartData, 0, Pop(metaDataHashMap, "filtertype", "peak"));
				}
				if (metaDataHashMap.contains("pfiltergain"))
				{
					chartData.audio.audioEffect.laser.paramChange["peaking_filter"]["gain"].emplace(0, std::to_string(PopInt<std::int32_t>(metaDataHashMap, "pfiltergain", 50)) + "%");
				}
				chartData.audio.audioEffect.laser.peakingFilterDelay = chartData.audio.bgm.legacy.filenameP.empty() ? PopInt<std::int32_t>(metaDataHashMap, "pfilterdelay", 40) : 0;
			}

			if constexpr (std::is_same_v<ChartDataType, ChartData>)
			{
				// "bg"
				const std::string bgStr = Pop(metaDataHashMap, "bg", "desert");
				if (bgStr.find(';') != std::string::npos)
				{
					const auto bgFilenames = Split<2>(bgStr, ';');
					chartData.bg.legacy.bg[0].filename = bgFilenames[0];
					chartData.bg.legacy.bg[1].filename = bgFilenames[1];
				}
				else
				{
					chartData.bg.legacy.bg[0].filename = bgStr;
					chartData.bg.legacy.bg[1].filename = bgStr;
				}

				// "layer"
				const char layerSeparator = (kshVersionInt >= 166) ? ';' : '/';
				const std::string layerStr = Pop(metaDataHashMap, "layer", "arrow");
				const auto layerOptionArray = Split<3>(layerStr, layerSeparator);
				chartData.bg.legacy.layer.filename = layerOptionArray[0];
				chartData.bg.legacy.layer.duration = ParseNumeric<std::int32_t>(layerOptionArray[1]);
				const std::int32_t rotationFlags = ParseNumeric<std::int32_t>(layerOptionArray[1], kRotationFlagTilt | kRotationFlagSpin);
				chartData.bg.legacy.layer.rotation = {
					.tilt = ((rotationFlags & kRotationFlagTilt) != 0),
					.spin = ((rotationFlags & kRotationFlagSpin) != 0),
				};

				chartData.bg.legacy.movie.filename = Pop(metaDataHashMap, "v");
				chartData.bg.legacy.movie.offset = PopInt<std::int32_t>(metaDataHashMap, "vo");

				chartData.gauge.total = PopInt<std::int32_t>(metaDataHashMap, "total", 0);
			}

			chartData.meta.information = Pop(metaDataHashMap, "information");
		}

		// Store unrecognized meta data in compat.meta
		if constexpr (std::is_same_v<ChartDataType, ChartData>)
		{
			for (const auto& [key, value] : metaDataHashMap)
			{
				chartData.compat.kshUnknown.meta.emplace(key, value);
			}
		}

		return chartData;
	}
}

MetaChartData kson::LoadKSHMetaChartData(std::istream& stream)
{
	return CreateChartDataFromMetaDataStream<MetaChartData>(stream, nullptr);
}

MetaChartData kson::LoadKSHMetaChartData(const std::string& filePath)
{
	if (!std::filesystem::exists(filePath))
	{
		return {
			.error = Error::kFileNotFound,
		};
	}

	std::ifstream ifs(filePath, std::ios_base::binary);
	if (!ifs.good())
	{
		return {
			.error = Error::kCannotOpenFileStream,
		};
	}

	MetaChartData chartData = LoadKSHMetaChartData(ifs);
	chartData.filePath = filePath;

	return chartData;
}

kson::ChartData kson::LoadKSHChartData(std::istream& stream)
{
	// Load chart meta data
	bool isUTF8;
	ChartData chartData = CreateChartDataFromMetaDataStream<ChartData>(stream, &isUTF8);

	assert(chartData.beat.timeSig.contains(0));
	TimeSig currentTimeSig = chartData.beat.timeSig.at(0);

	double currentTempo = 120.0;
	if (chartData.beat.bpm.contains(0))
	{
		currentTempo = chartData.beat.bpm.at(0);
	}

	const std::int32_t kshVersionInt = ParseNumeric<std::int32_t>(chartData.compat.kshVersion, 170);

	// For backward compatibility of zoom_top/zoom_bottom/zoom_side
	const double zoomAbsMax = (kshVersionInt >= 167) ? kZoomAbsMax : kZoomAbsMaxLegacy;
	const std::size_t zoomMaxChar = (kshVersionInt >= 167) ? kZoomMaxChar : kZoomMaxCharLegacy;

	// Buffers
	// (needed because actual addition cannot come before the pulse value calculation)
	std::vector<std::string> chartLines;
	std::vector<BufOptionLine> optionLines;
	std::vector<BufCommentLine> commentLines;
	std::vector<BufUnknownLine> unknownLines;
	ByPulse<std::int32_t> relScrollSpeeds;
	PreparedLongNoteArray preparedLongNoteArray(&chartData);

	// GraphSections buffers
	PreparedGraphSection preparedManualTilt(&chartData);

	// Note option buffers (key: chart line index)
	std::array<std::unordered_set<std::size_t>, kNumLaserLanes> currentMeasureLaserXScale2x;
	std::array<std::unordered_map<std::size_t, std::string>, kNumFXLanes> currentMeasureFXAudioEffectStrs; // "fx-l=" or "fx-r=" in KSH
	std::array<std::unordered_map<std::size_t, std::string>, kNumFXLanes> currentMeasureFXAudioEffectParamStrs; // "fx-l_param1=" or "fx-r_param1=" in KSH
	std::array<std::unordered_map<std::size_t, BufKeySound>, kNumFXLanes> currentMeasureFXKeySounds; // "fx-l_se=" or "fx-r_se=" in KSH
	std::unordered_map<std::size_t, std::string> currentMeasureLaserKeySounds; // "chokkakuse=" in KSH

	Pulse currentPulse = 0;
	std::int64_t currentMeasureIdx = 0;

	// Read chart body
	// The stream start from the next of the first bar line ("--")
	std::string line;
	while (std::getline(stream, line, '\n'))
	{
		// Eliminate CR
		if (!line.empty() && *line.crbegin() == '\r')
		{
			line.pop_back();
		}

		// Skip empty lines
		if (line.empty())
		{
			continue;
		}

		// Comments
		if (IsCommentLine(line))
		{
			commentLines.push_back({
				.lineIdx = chartLines.size(),
				.value = line.substr(2), // 2 = strlen("//")
			});
			continue;
		}

		// User-defined audio effects
		if (line[0] == '#')
		{
			const bool isDefineFX = line.starts_with("#define_fx ");
			const bool isDefineFilter = !isDefineFX && line.starts_with("#define_filter ");
			if (isDefineFX || isDefineFilter)
			{
				std::string_view sv = line;

				// Move cursor to audio effect name start
				{
					bool whiteSpaceFound = false;
					while (!sv.empty())
					{
						if (sv[0] == ' ')
						{
							whiteSpaceFound = true;
						}
						else if (whiteSpaceFound)
						{
							break;
						}

						sv = sv.substr(1);
					}
				}

				// Get audio effect name while moving cursor to parameter value start
				std::string name;
				{
					bool whiteSpaceFound = false;
					while (!sv.empty())
					{
						if (sv[0] == ' ')
						{
							whiteSpaceFound = true;
						}
						else if (whiteSpaceFound)
						{
							break;
						}
						else
						{
							name.push_back(sv[0]);
						}

						sv = sv.substr(1);
					}
				}

				Dict<std::string> params;
				while (!sv.empty())
				{
					const std::size_t semicolonIdx = sv.find_first_of(kAudioEffectStrSeparator);
					std::string_view paramSV = (semicolonIdx == std::string_view::npos) ? sv : sv.substr(0, semicolonIdx);
					const std::pair<std::string, std::string> pair = SplitOptionLine(paramSV, isUTF8);
					if (!pair.second.empty())
					{
						params.emplace(pair);
					}

					if (semicolonIdx == std::string_view::npos)
					{
						break;
					}
					else
					{
						sv = sv.substr(semicolonIdx + 1);
					}
				}

				assert(params.contains("type"));
				if (!params.contains("type"))
				{
					continue;
				}

				const std::string type = params.at("type");
				params.erase("type");
				assert(s_audioEffectTypeTable.contains(type));
				if (!s_audioEffectTypeTable.contains(type))
				{
					continue;
				}

				AudioEffectParams paramsKSON;
				for (const auto& [paramName, value] : params)
				{
					// Note: Parameters deleted in the KSON format (e.g., "hiCutGain") are just ignored.
					if (s_audioEffectParamNameTable.contains(paramName))
					{
						paramsKSON.emplace(s_audioEffectParamNameTable.at(paramName), value);
					}
				}

				auto& def = isDefineFX ? chartData.audio.audioEffect.fx.def : chartData.audio.audioEffect.laser.def;
				def.emplace(name, AudioEffectDef{
					.type = s_audioEffectTypeTable.at(type),
					.v = std::move(paramsKSON),
				});
			}
			continue;
		}

		if (IsChartLine(line))
		{
			chartLines.push_back(line);
			continue;
		}

		if (IsOptionLine(line))
		{
			const auto [key, value] = SplitOptionLine(line, isUTF8);
			if (key == "t")
			{
				if (value.find('-') == std::string::npos)
				{
					currentTempo = ParseNumeric<double>(value);
				}
				optionLines.push_back({
					.lineIdx = chartLines.size(),
					.key = key,
					.value = value,
				});
			}
			else if (key == "beat")
			{
				currentTimeSig = ParseTimeSig(value);
				chartData.beat.timeSig.emplace(currentMeasureIdx, currentTimeSig);
			}
			else
			{
				optionLines.push_back({
					.lineIdx = chartLines.size(),
					.key = key,
					.value = value,
				});
			}
			continue;
		}
		
		if (IsBarLine(line))
		{
			const std::size_t bufLineCount = chartLines.size();
			const RelPulse oneLinePulse = kResolution4 * currentTimeSig.numerator / currentTimeSig.denominator / bufLineCount;

			// Add options that require their position
			for (const auto& [lineIdx, key, value] : optionLines)
			{
				const Pulse time = currentPulse + lineIdx * oneLinePulse;
				if (key == "t")
				{
					if (chartData.beat.bpm.empty()) [[unlikely]]
					{
						// In rare cases where BPM is not specified on the chart metadata
						InsertBPMChange(chartData.beat.bpm, 0, value);
					}
					else
					{
						InsertBPMChange(chartData.beat.bpm, time, value);
					}
				}
				else if (key == "stop")
				{
					if (relScrollSpeeds.contains(time))
					{
						relScrollSpeeds.at(time) -= 1;
					}
					else
					{
						relScrollSpeeds.emplace(time, -1);
					}

					const Pulse endTime = time + KSHLengthToRelPulse(value);
					if (relScrollSpeeds.contains(endTime))
					{
						relScrollSpeeds.at(endTime) += 1;
					}
					else
					{
						relScrollSpeeds.emplace(endTime, 1);
					}
				}
				else if (key == "zoom_top")
				{
					const double dValue = ParseNumeric<double>(std::string_view(value).substr(0, zoomMaxChar)) * kKSHToKSONCamScale;
					if (std::abs(dValue) <= zoomAbsMax || (kshVersionInt < 167 && chartData.camera.cam.body.rotationX.contains(time)))
					{
						chartData.camera.cam.body.rotationX.insert_or_assign(time, dValue);
					}
				}
				else if (key == "zoom_bottom")
				{
					const double dValue = ParseNumeric<double>(std::string_view(value).substr(0, zoomMaxChar)) * kKSHToKSONCamScale;
					if (std::abs(dValue) <= zoomAbsMax || (kshVersionInt < 167 && chartData.camera.cam.body.zoom.contains(time)))
					{
						chartData.camera.cam.body.zoom.insert_or_assign(time, dValue);
					}
				}
				else if (key == "zoom_side")
				{
					const double dValue = ParseNumeric<double>(std::string_view(value).substr(0, zoomMaxChar)) * kKSHToKSONCamScale;
					if (std::abs(dValue) <= zoomAbsMax || (kshVersionInt < 167 && chartData.camera.cam.body.shiftX.contains(time)))
					{
						chartData.camera.cam.body.shiftX.insert_or_assign(time, dValue);
					}
				}
				else if (key == "center_split")
				{
					const double dValue = ParseNumeric<double>(value) * kKSHToKSONCamScale;
					if (std::abs(dValue) <= kCenterSplitAbsMax)
					{
						chartData.camera.cam.body.centerSplit.insert_or_assign(time, dValue);
					}
				}
				else if (key == "tilt")
				{
					if (IsTiltValueManual(value))
					{
						const double dValue = ParseNumeric<double>(value) * kKSHToKSONTiltScale;
						if (std::abs(dValue) <= kManualTiltAbsMax)
						{
							preparedManualTilt.prepare(time);
							preparedManualTilt.addGraphPoint(time, dValue);
						}
					}
					else
					{
						if (preparedManualTilt.prepared())
						{
							preparedManualTilt.publishManualTilt();
						}

						// Insert tilt.scale
						if (s_tiltTypeScaleTable.contains(value))
						{
							auto& target = chartData.camera.tilt.scale;
							const double prevValue = target.empty() ? 1.0 : target.crbegin()->second;
							const double currentValue = s_tiltTypeScaleTable.at(value);
							if (currentValue != prevValue)
							{
								target.insert_or_assign(time, currentValue);
							}
						}

						// Insert tilt.keep
						{
							auto& target = chartData.camera.tilt.keep;
							const bool prevValue = target.empty() ? false : target.crbegin()->second;
							const bool currentValue = value.starts_with("keep_");
							if (currentValue != prevValue)
							{
								target.insert_or_assign(time, currentValue);
							}
						}
					}
				}
				else if (key == "chokkakuvol")
				{
					const double dValue = ParseNumeric<double>(value) / 100;
					chartData.audio.keySound.laser.vol.insert_or_assign(time, dValue);
				}
				else if (key == "chokkakuse")
				{
					currentMeasureLaserKeySounds.insert_or_assign(lineIdx, value);
				}
				else if (key == "pfiltergain")
				{
					chartData.audio.audioEffect.laser.paramChange["peaking_filter"]["gain"].emplace(time, std::to_string(ParseNumeric<std::int32_t>(value, 50)) + "%");
				}
				else if (key == "fx-l")
				{
					currentMeasureFXAudioEffectStrs[0].insert_or_assign(lineIdx, value);
				}
				else if (key == "fx-r")
				{
					currentMeasureFXAudioEffectStrs[1].insert_or_assign(lineIdx, value);
				}
				// Note: "fx-l_param2"/"fx-r_param2" need not be processed because "fx-l_param1"/"fx-r_param1" is legacy (< v1.60) and 
				//       Echo, the only audio effect that uses a second parameter, was added in v1.60.
				else if (key == "fx-l_param1")
				{
					currentMeasureFXAudioEffectParamStrs[0].insert_or_assign(lineIdx, value);
				}
				else if (key == "fx-r_param1")
				{
					currentMeasureFXAudioEffectParamStrs[1].insert_or_assign(lineIdx, value);
				}
				else if (bool isL = key == "fx-l_se"; isL || key == "fx-r_se")
				{
					const auto strPair = Split<2>(value, ';');
					currentMeasureFXKeySounds[isL ? 0 : 1].insert_or_assign(lineIdx, BufKeySound{
						.name = strPair[0],
						.vol = ParseNumeric<std::int32_t>(strPair[1], 50),
					});
				}
				else if (key == "filtertype")
				{
					InsertFiltertype(chartData, time, value);
				}
				else if (key == "laserrange_l")
				{
					if (value == "2x")
					{
						currentMeasureLaserXScale2x[0].emplace(lineIdx);
					}
				}
				else if (key == "laserrange_r")
				{
					if (value == "2x")
					{
						currentMeasureLaserXScale2x[1].emplace(lineIdx);
					}
				}
				else if (bool isFX = key.starts_with("fx:"); isFX || key.starts_with("filter:"))
				{
					constexpr std::size_t kAudioEffectNameIdx = 1;
					constexpr std::size_t kParamNameIdx = 2;

					const auto& a = Split<3>(key, ':');
					if (!a[kAudioEffectNameIdx].empty() && !a[kParamNameIdx].empty())
					{
						auto& paramChange = isFX ? chartData.audio.audioEffect.fx.paramChange : chartData.audio.audioEffect.laser.paramChange;
						if (s_audioEffectParamNameTable.contains(a[kParamNameIdx]))
						{
							paramChange[a[kAudioEffectNameIdx]][std::string(s_audioEffectParamNameTable.at(a[kParamNameIdx]))].insert_or_assign(time, value);
						}
					}
				}
				else
				{
					chartData.compat.kshUnknown.option[key].emplace(time, value);
				}
			}

			// Add notes
			for (std::size_t i = 0; i < bufLineCount; ++i)
			{
				const std::string_view buf = chartLines.at(i);
				std::size_t currentBlock = 0;
				std::size_t laneIdx = 0;

				const Pulse time = currentPulse + i * oneLinePulse;

				for (std::size_t j = 0; j < buf.size(); ++j)
				{
					if (buf[j] == kBlockSeparator)
					{
						++currentBlock;
						laneIdx = 0;
						continue;
					}

					if (currentBlock == kBlockIdxBT && laneIdx < kNumBTLanes) // BT notes
					{
						auto& preparedLongNoteRef = preparedLongNoteArray.bt[laneIdx];
						switch (buf[j])
						{
						case '2': // Long BT note
							if (!preparedLongNoteRef.prepared())
							{
								preparedLongNoteRef.prepare(time);
							}
							preparedLongNoteRef.extendLength(oneLinePulse);
							break;
						case '1': // Chip BT note
							preparedLongNoteRef.publishLongBTNote();
							chartData.note.bt[laneIdx].emplace(time, 0);
							break;
						default:  // Empty
							preparedLongNoteRef.publishLongBTNote();
							break;
						}
					}
					else if (currentBlock == kBlockIdxFX && laneIdx < kNumFXLanes) // FX notes
					{
						auto& preparedLongNoteRef = preparedLongNoteArray.fx[laneIdx];
						switch (buf[j])
						{
						case '2': // Chip FX note
							chartData.note.fx[laneIdx].emplace(time, 0);
							if (currentMeasureFXKeySounds[laneIdx].contains(i))
							{
								const auto& bufKeySound = currentMeasureFXKeySounds[laneIdx].at(i);
								chartData.audio.keySound.fx.chipEvent[bufKeySound.name][laneIdx].emplace(time, KeySoundInvokeFX{
									.vol = static_cast<double>(bufKeySound.vol) / 100,
								});
							}
							break;
						case '0': // Empty
							preparedLongNoteRef.publishLongFXNote();
							break;
						case '1': // Long FX note
							if (currentMeasureFXAudioEffectStrs[laneIdx].contains(i))
							{
								const std::string audioEffectStr = currentMeasureFXAudioEffectStrs[laneIdx].at(i);
								const std::string audioEffectParamStr =
									currentMeasureFXAudioEffectParamStrs[laneIdx].contains(i)
									? currentMeasureFXAudioEffectParamStrs[laneIdx].at(i) // Note: Normally this is not used here because it's for legacy long FX chars
									: "";
								preparedLongNoteRef.prepare(time, audioEffectStr, audioEffectParamStr, false);
							}
							else
							{
								preparedLongNoteRef.prepare(time);
							}
							preparedLongNoteRef.extendLength(oneLinePulse);
							break;
						default: // Long FX note (legacy characters, e.g., "F" = Flanger)
							{
								const std::string audioEffectStr(KSHLegacyFXCharToKSHAudioEffectStr(buf[j]));
								const std::string audioEffectParamStr = currentMeasureFXAudioEffectParamStrs[laneIdx].contains(i) ? currentMeasureFXAudioEffectParamStrs[laneIdx].at(i) : "";
								preparedLongNoteRef.prepare(time, audioEffectStr, audioEffectParamStr, true);
							}
							preparedLongNoteRef.extendLength(oneLinePulse);
							break;
						}
					}
					else if (currentBlock == kBlockIdxLaser && laneIdx < kNumLaserLanes) // Laser notes
					{
						auto& preparedLaserSectionRef = preparedLongNoteArray.laser[laneIdx];
						switch (buf[j])
						{
						case '-': // Empty
							preparedLaserSectionRef.publishLaserNote();
							preparedLaserSectionRef.clear();
							break;
						case ':': // Connection
							break;
						default:
							{
								const std::int32_t laserX = CharToLaserX(buf[j]);
								if (laserX >= 0)
								{
									if (!preparedLaserSectionRef.prepared())
									{
										const std::int8_t scaleX = currentMeasureLaserXScale2x[laneIdx].contains(i) ? 2 : 1;
										preparedLaserSectionRef.prepare(time, scaleX);
									}

									const double dLaserX = static_cast<double>(laserX) / kLaserXMax;
									preparedLaserSectionRef.addGraphPoint(time, dLaserX);

									if (currentMeasureLaserKeySounds.contains(i))
									{
										// Note: Here, the key sound element is inserted even if the laser segment is not a slam, but it doesn't matter much.
										const std::string& name = currentMeasureLaserKeySounds.at(i);
										if (!name.empty())
										{
											chartData.audio.keySound.laser.slamEvent[name].insert(time);
										}
									}
								}
							}
							break;
						}
					}
					else if (currentBlock == kBlockIdxLaser && laneIdx == kNumLaserLanes) // Lane spin
					{
						// Create a lane spin from string
						const PreparedLaneSpin laneSpin = PreparedLaneSpin::FromKSHSpinStr(buf.substr(j));
						if (laneSpin.isValid())
						{
							// Assign to the laser point if valid
							for (auto& lane : preparedLongNoteArray.laser)
							{
								lane.addLaneSpin(time, laneSpin);
							}
						}
					}
					++laneIdx;
				}
			}

			// Add comments
			for (const auto& [lineIdx, value] : commentLines)
			{
				const Pulse time = currentPulse + lineIdx * oneLinePulse;
				chartData.editor.comment.emplace(time, value);
			}

			// Add unknown lines
			for (const auto& [lineIdx, value] : unknownLines)
			{
				const Pulse time = currentPulse + lineIdx * oneLinePulse;
				chartData.compat.kshUnknown.line.emplace(time, value);
			}

			chartLines.clear();
			optionLines.clear();
			commentLines.clear();
			unknownLines.clear();
			for (auto& set : currentMeasureLaserXScale2x)
			{
				set.clear();
			}
			for (auto& map : currentMeasureFXAudioEffectStrs)
			{
				map.clear();
			}
			for (auto& map : currentMeasureFXAudioEffectParamStrs)
			{
				map.clear();
			}
			currentPulse += kResolution4 * currentTimeSig.numerator / currentTimeSig.denominator;
			++currentMeasureIdx;
			continue;
		}

		// Insert unrecognized line
		unknownLines.push_back({
			.lineIdx = chartLines.size(),
			.value = line,
		});
	}

	// Publish the last manual tilt section if exists
	if (preparedManualTilt.prepared())
	{
		preparedManualTilt.publishManualTilt();
	}

	// KSH file must end with the bar line "--" (except for user-defined audio effects), so there can never be a prepared button note here
	for (const auto& preparedBTNote : preparedLongNoteArray.bt)
	{
		assert(!preparedBTNote.prepared());
	}
	for (const auto& preparedFXNote : preparedLongNoteArray.fx)
	{
		assert(!preparedFXNote.prepared());
	}

	// The prepared laser section is published only when the laser lane is blank ("-"), so there can be unpublished laser sections here
	for (auto& preparedFXSection : preparedLongNoteArray.laser)
	{
		preparedFXSection.publishLaserNote();
	}

	// Convert scroll speeds
	{
		std::int32_t currentSpeed = 1;
		for (const auto& [y, relSpeed] : relScrollSpeeds)
		{
			if (y < 0) [[unlikely]]
			{
				continue;
			}

			currentSpeed += relSpeed;
			chartData.beat.scrollSpeed.emplace(y, static_cast<double>(currentSpeed));
		}

		if (!chartData.beat.scrollSpeed.contains(0))
		{
			chartData.beat.scrollSpeed.emplace(0, 1.0);
		}
	}

	// Convert FX parameters
	for (auto& [audioEffectName, lanes] : chartData.audio.audioEffect.fx.longEvent)
	{
		AudioEffectType type = AudioEffectType::Unspecified;
		if (chartData.audio.audioEffect.fx.def.contains(audioEffectName))
		{
			// User-defined audio effects
			type = chartData.audio.audioEffect.fx.def.at(audioEffectName).type;
		}
		else
		{
			// Preset audio effects
			type = StrToAudioEffectType(audioEffectName);
		}

		assert(type != AudioEffectType::Unspecified || audioEffectName.empty());

		if (type == AudioEffectType::Unspecified)
		{
			for (auto& lane : lanes)
			{
				for (auto& [y, params] : lane)
				{
					params.erase("_param1");
					params.erase("_param2");
				}
			}
		}
		else
		{
			for (auto& lane : lanes)
			{
				for (auto& [y, params] : lane)
				{
					// Convert temporary stored "_param1"/"_param2" values to parameter values for each audio effect type
					if (params.contains("_param1") && params.contains("_param2"))
					{
						std::string param1 = params.at("_param1");
						std::string param2 = params.at("_param2");

						// Replace unspecified values with defaults
						if (param1 == kAudioEffectParamUnspecifiedStr)
						{
							switch (type)
							{
							case AudioEffectType::Retrigger:
								param1 = "8";
								break;
							case AudioEffectType::Gate:
								param1 = "4";
								break;
							case AudioEffectType::Wobble:
								param1 = "12";
								break;
							case AudioEffectType::PitchShift:
								param1 = "12";
								break;
							case AudioEffectType::Bitcrusher:
								param1 = "5";
								break;
							case AudioEffectType::Tapestop:
								param1 = "50";
								break;
							case AudioEffectType::Echo:
								param1 = "4";
								break;
							default:
								param1 = "0";
								break;
							};
						}
						if (param2 == kAudioEffectParamUnspecifiedStr)
						{
							switch (type)
							{
							case AudioEffectType::Echo:
								param2 = "60";
								break;
							default:
								param2 = "0";
								break;
							};
						}

						// Insert parameter
						switch (type)
						{
						case AudioEffectType::Retrigger:
						case AudioEffectType::Gate:
						case AudioEffectType::Wobble:
							if (ParseNumeric<std::int32_t>(param1) > 0)
							{
								params.emplace("wave_length", "1/" + param1);
							}
							break;
						case AudioEffectType::PitchShift:
							params.emplace("pitch", param1);
							break;
						case AudioEffectType::Bitcrusher:
							params.emplace("reduction", param1 + "samples");
							break;
						case AudioEffectType::Tapestop:
							params.emplace("speed", param1 + "%");
							break;
						case AudioEffectType::Echo:
							if (ParseNumeric<std::int32_t>(param1) > 0)
							{
								params.emplace("wave_length", "1/" + param1);
							}
							params.emplace("feedback", param2 + "%");
							break;
						};

						params.erase("_param1");
						params.erase("_param2");
					}
				}
			}
		}
	}

	return chartData;
}

ChartData kson::LoadKSHChartData(const std::string& filePath)
{
	if (!std::filesystem::exists(filePath))
	{
		return {
			.error = Error::kFileNotFound,
		};
	}

	std::ifstream ifs(filePath, std::ios_base::binary);
	if (!ifs.good())
	{
		return {
			.error = Error::kCannotOpenFileStream,
		};
	}

	ChartData chartData = LoadKSHChartData(ifs);
	chartData.filePath = filePath;

	return chartData;
}
