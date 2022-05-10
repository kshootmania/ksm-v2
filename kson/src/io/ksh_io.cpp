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

		return {
			optionLineUTF8.substr(0, equalIdx),
			optionLineUTF8.substr(equalIdx + 1)
		};
	}

	std::tuple<std::string, std::int64_t, std::int64_t> SplitAudioEffectStr(std::string_view optionLine)
	{
		// TODO: default values of params
		using Tuple = std::tuple<std::string, std::int64_t, std::int64_t>;

		const std::size_t semicolonIdx1 = optionLine.find_first_of(kAudioEffectStrSeparator);
		if (semicolonIdx1 == std::string_view::npos)
		{
			return Tuple{ optionLine, 0, 0 };
		}

		const std::size_t semicolonIdx2 = optionLine.substr(semicolonIdx1 + 1).find_first_of(kAudioEffectStrSeparator);
		const std::int64_t param1 = ParseNumeric<std::int64_t>(optionLine.substr(semicolonIdx1 + 1));
		if (semicolonIdx2 == std::string_view::npos)
		{
			return Tuple{ optionLine.substr(0, semicolonIdx1), param1, 0 };
		}

		const std::int64_t param2 = ParseNumeric<std::int64_t>(optionLine.substr(semicolonIdx1 + semicolonIdx2 + 2));

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

	const std::unordered_map<std::string_view, std::string_view> s_kshFXToKsonAudioEffectNameTable
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
		{ "SwitchAudio", "audio_swap" },
	};

	const std::unordered_map<std::string_view, std::string_view> s_kshFilterToKsonAudioEffectNameTable
	{
		{ "peak", "peaking_filter" },
		{ "hpf1", "high_pass_filter" },
		{ "lpf1", "low_pass_filter" },
		{ "bitc", "bitcrusher" },
		// TODO: Add fallback effect option to audio_swap to simulate "fx;bitc"
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

	// TODO: refactor
	class AbstractPreparedLongNote
	{
	protected:
		bool m_prepared = false;

		Pulse m_time = 0;

		RelPulse m_length = 0;

		ChartData* m_pTargetChartData = nullptr;

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

			m_pTargetChartData->note.btLanes[m_targetLaneIdx].emplace(
				m_time,
				m_length);

			clear();
		}
	};

	class PreparedLongFXNote : public AbstractPreparedLongNote
	{
	protected:
		// FX audio effect string ("fx-l=" or "fx-r=" in .ksh)
		std::string m_audioEffectStr;

		// Legacy FX audio effect parameters ("fx-l_param1=" or "fx-r_param1=" in .ksh)
		std::string m_audioEffectParamStr;

	public:
		PreparedLongFXNote() = default;

		PreparedLongFXNote(ChartData* pTargetChartData, std::size_t targetLaneIdx)
			: AbstractPreparedLongNote(pTargetChartData, targetLaneIdx)
		{
		}

		virtual ~PreparedLongFXNote() = default;

		void prepare(Pulse time) = delete;

		void prepare(Pulse time, std::string_view audioEffectStr, std::string_view audioEffectParamStr)
		{
#ifdef KSM_EDITOR
			if (m_prepared && (audioEffectStr != m_audioEffectStr || audioEffectParamStr != m_audioEffectParamStr))
			{
				publishLongFXNote();
			}
#endif
			if (!m_prepared)
			{
				AbstractPreparedLongNote::prepare(time);
				m_audioEffectStr = audioEffectStr;
				m_audioEffectParamStr = audioEffectParamStr;
			}
		}

		void publishLongFXNote()
		{
			if (!m_prepared)
			{
				return;
			}

			auto& targetLane = m_pTargetChartData->note.fxLanes[m_targetLaneIdx];

			// Publish prepared long FX note
			const auto [_, inserted] = targetLane.emplace(
				m_time,
				m_length);

			if (inserted)
			{
				auto [audioEffectName, audioEffectParamValue1, audioEffectParamValue2] = SplitAudioEffectStr(m_audioEffectStr);
				if (!m_audioEffectParamStr.empty())
				{
					// Apply legacy FX audio effect parameters
					audioEffectParamValue1 = ParseNumeric<std::int64_t>(m_audioEffectParamStr);
				}
				if (s_kshFXToKsonAudioEffectNameTable.contains(audioEffectName))
				{
					// Convert the name of preset audio effects
					audioEffectName = s_kshFXToKsonAudioEffectNameTable.at(audioEffectName);
				}
				if (!audioEffectName.empty())
				{
					m_pTargetChartData->audio.audioEffects.fx.longInvoke[audioEffectName][m_targetLaneIdx].emplace(m_time, AudioEffectParams{
						// Store the value of the parameters in temporary keys
						// (Since the conversion requires determining the type of audio effect, it is processed
						//  after reading the "#define_fx"/"#define_filter" lines.)
						{ "_param1", AudioEffectParam(static_cast<double>(audioEffectParamValue1)) },
						{ "_param2", AudioEffectParam(static_cast<double>(audioEffectParamValue2)) },
					});
				}
			}

			clear();
		}

		virtual void clear() override
		{
			AbstractPreparedLongNote::clear();
			m_audioEffectStr.clear();
			m_audioEffectParamStr.clear();
		}
	};

	Pulse KSHLengthToMeasure(std::string_view str, Pulse resolution)
	{
		return ParseNumeric<Pulse>(str) * resolution * 4 / 192;
	}

	std::tuple<Pulse, std::int64_t, std::int64_t, std::int64_t> SplitSwingParams(std::string_view paramStr, Pulse resolution)
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
			KSHLengthToMeasure(params[0], resolution),
			ParseNumeric<std::int64_t>(params[1]),
			ParseNumeric<std::int64_t>(params[2]),
			ParseNumeric<std::int64_t>(params[3]));
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

		Pulse duration = 0;

		std::int64_t swingAmplitude = 0;

		std::int64_t swingRepeat = 0;

		std::int64_t swingDecayOrder = 0;

		static PreparedLaneSpin FromKSHSpinStr(std::string_view strFromKsh, Pulse resolution) // From .ksh spin string (example: "@(192")
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
			Pulse duration;
			std::int64_t swingAmplitude = 0, swingRepeat = 0, swingDecayOrder = 0;
			if (type == Type::kNoSpin || direction == Direction::kUnspecified)
			{
				duration = 0;
			}
			else if (type == Type::kSwing)
			{
				std::tie(duration, swingAmplitude, swingRepeat, swingDecayOrder) = SplitSwingParams(strFromKsh.substr(2), resolution);
			}
			else
			{
				duration = KSHLengthToMeasure(strFromKsh.substr(2), resolution);
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
		{ PreparedLaneSpin::Type::kNormal, "spin" },
		{ PreparedLaneSpin::Type::kHalf, "half_spin" },
		{ PreparedLaneSpin::Type::kSwing, "swing" },
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

	class PreparedLaserSection : public PreparedGraphSection
	{
	private:
		std::size_t m_targetLaneIdx = 0;
		std::int8_t m_xScale = 1;
		ByRelPulse<PreparedLaneSpin> m_preparedLaneSpins;

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

		void prepare(Pulse time, std::int8_t xScale)
		{
			if (!m_prepared)
			{
				PreparedGraphSection::prepare(time);
				m_xScale = xScale;
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
			const Pulse laserSlamThreshold = m_pTargetChartData->beat.resolution * 4 / 32;
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
			auto& targetLane = m_pTargetChartData->note.laserLanes[m_targetLaneIdx];
			const auto [_, inserted] = targetLane.emplace(
				m_time,
				LaserSection{
					.points = convertedGraphSection,
					.xScale = m_xScale,
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
								.decayOrder = static_cast<double>(laneSpin.swingDecayOrder),
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
							m_pTargetChartData->camera.cam.pattern.laser.slamInvoke[patternKey].emplace(m_time + relPulse, params);
						}
					}
				}
			}

			clear();
		}

		virtual void clear() override
		{
			PreparedGraphSection::clear();

			m_xScale = 1;
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

			const std::string difficultyName = Pop(metaDataHashMap, "difficulty", "infinite");
			if (s_difficultyNameTable.contains(difficultyName))
			{
				chartData.meta.difficulty.idx = s_difficultyNameTable.at(difficultyName);
			}
			else
			{
				chartData.meta.difficulty.idx = 3;
			}

			chartData.meta.level = PopInt<std::int8_t>(metaDataHashMap, "level", 1, 1, 20);

			if constexpr (std::is_same_v<ChartDataType, ChartData>)
			{
				// Insert the first time signature change
				TimeSig firstTimeSig{ 4, 4 };
				if (metaDataHashMap.contains("beat")) [[unlikely]] // unlikely because "beat=" is usually after the first bar line
				{
					firstTimeSig = ParseTimeSig(metaDataHashMap.at("beat"));
					metaDataHashMap.erase("beat");
				}
				chartData.beat.timeSigChanges.emplace(0, firstTimeSig);

				// Insert the first tempo change
				if (metaDataHashMap.contains("t")) [[likely]]
				{
					InsertBPMChange(chartData.beat.bpmChanges, 0, metaDataHashMap.at("t"));
				}
			}
			chartData.meta.dispBPM = Pop(metaDataHashMap, "t", "");

			chartData.meta.standardBPM = ParseNumeric<double>(Pop(metaDataHashMap, "to", "0"), 0.0);

			const auto bgmFilenames = Split<4>(Pop(metaDataHashMap, "m"), ';');
			if constexpr (std::is_same_v<ChartDataType, ChartData>)
			{
				chartData.audio.bgmInfo.filename = bgmFilenames[0];
				chartData.audio.legacy.bgmInfo.filenameF = bgmFilenames[1];
				chartData.audio.legacy.bgmInfo.filenameP = bgmFilenames[2];
				chartData.audio.legacy.bgmInfo.filenameFP = bgmFilenames[3];
			}
			chartData.audio.bgmInfo.preview.filename = bgmFilenames[0];

			const std::int32_t volInt = PopInt<std::int32_t>(metaDataHashMap, "mvol", 50);
			chartData.audio.bgmInfo.vol = volInt / 100.0;
			if (chartData.compat.kshVersion.empty())
			{
				// For historical reasons, the value is multiplied by 0.6 if the value of "ver" is not specified.
				chartData.audio.bgmInfo.vol *= 0.6;
			}

			// TODO: Store chokkakuautovol
			Pop(metaDataHashMap, "chokkakuautovol", "1");

			if constexpr (std::is_same_v<ChartDataType, ChartData>)
			{
				chartData.audio.bgmInfo.offset = PopInt<std::int64_t>(metaDataHashMap, "o", 0);
			}
			chartData.audio.bgmInfo.preview.offset = PopInt<std::int64_t>(metaDataHashMap, "po", 0);
			chartData.audio.bgmInfo.preview.duration = PopInt<std::int64_t>(metaDataHashMap, "plength", 0);

			if constexpr (std::is_same_v<ChartDataType, ChartData>)
			{
				// "bg"
				const std::string bgStr = Pop(metaDataHashMap, "bg", "desert");
				if (bgStr.find(';') != std::string::npos)
				{
					const auto bgFilenames = Split<2>(bgStr, ';');
					chartData.bg.legacy.bgInfos[0].filename = bgFilenames[0];
					chartData.bg.legacy.bgInfos[1].filename = bgFilenames[1];
				}
				else
				{
					chartData.bg.legacy.bgInfos[0].filename = bgStr;
					chartData.bg.legacy.bgInfos[1].filename = bgStr;
				}

				// "layer"
				const char layerSeparator = (kshVersionInt >= 166) ? ';' : '/';
				const std::string layerStr = Pop(metaDataHashMap, "layer", "arrow");
				const auto layerOptionArray = Split<3>(layerStr, layerSeparator);
				auto& layerInfos = chartData.bg.legacy.layerInfos;
				layerInfos[0].filename = layerInfos[1].filename = layerOptionArray[0];
				layerInfos[0].durationMs = layerInfos[1].durationMs = ParseNumeric<int64_t>(layerOptionArray[1]);
				const std::int32_t rotationFlagsInt = ParseNumeric<std::int32_t>(layerOptionArray[1], 3);
				layerInfos[0].rotationFlags = layerInfos[1].rotationFlags = {
					.tiltAffected = ((rotationFlagsInt & 1) != 0),
					.spinAffected = ((rotationFlagsInt & 2) != 0),
				};

				chartData.bg.legacy.movieInfos.filename = Pop(metaDataHashMap, "v");
				chartData.bg.legacy.movieInfos.offsetMs = PopInt<std::int64_t>(metaDataHashMap, "vo");

				chartData.gauge.total = static_cast<double>(PopInt<std::int32_t>(metaDataHashMap, "total", 0));
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

	assert(chartData.beat.timeSigChanges.contains(0));
	TimeSig currentTimeSig = chartData.beat.timeSigChanges.at(0);

	double currentTempo = 120.0;
	if (chartData.beat.bpmChanges.contains(0))
	{
		currentTempo = chartData.beat.bpmChanges.at(0);
	}

	const std::int32_t kshVersionInt = ParseNumeric<std::int32_t>(chartData.compat.kshVersion, 170);

	// For backward compatibility of zoom_top/zoom_bottom/zoom_side
	const double zoomAbsMax = (kshVersionInt >= 167) ? kZoomAbsMax : kZoomAbsMaxLegacy;
	const std::size_t zoomMaxChar = (kshVersionInt >= 167) ? kZoomMaxChar : kZoomMaxCharLegacy;

	// Buffers
	// (needed because actual addition cannot come before the pulse value calculation)
	std::vector<std::string> chartLines;
	std::vector<BufOptionLine> optionLines;
	PreparedLongNoteArray preparedLongNoteArray(&chartData);

	// GraphSections buffers
	PreparedGraphSection preparedManualTilt(&chartData);

	// Note option buffers (key: chart line index)
	std::array<std::unordered_set<std::size_t>, kNumLaserLanes> currentMeasureLaserXScale2x;
	std::array<std::unordered_map<std::size_t, std::string>, kNumFXLanes> currentMeasureFXAudioEffectStrs; // "fx-l=" or "fx-r=" in KSH
	std::array<std::unordered_map<std::size_t, std::string>, kNumFXLanes> currentMeasureFXAudioEffectParamStrs; // "fx-l_param1=" or "fx-r_param1=" in KSH

	Pulse currentPulse = 0;
	std::int64_t currentMeasureIdx = 0;

	// TODO: chokkakuvol (laser slam volume envelope)

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

		// Skip comments
		if (IsCommentLine(line))
		{
			continue;
		}

		// TODO: Read user-defined audio effects
		if (!line.empty() && line[0] == '#')
		{
			continue;
		}

		if (IsChartLine(line))
		{
			chartLines.push_back(line);
		}
		else if (IsOptionLine(line))
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
				chartData.beat.timeSigChanges.emplace(currentMeasureIdx, currentTimeSig);
			}
			else
			{
				optionLines.push_back({
					.lineIdx = chartLines.size(),
					.key = key,
					.value = value,
				});
			}
		}
		else if (IsBarLine(line))
		{
			const std::size_t bufLineCount = chartLines.size();
			const Pulse oneLinePulse = chartData.beat.resolution * 4 * currentTimeSig.numerator / currentTimeSig.denominator / bufLineCount;

			// Add options that require their position
			for (const auto& [lineIdx, key, value] : optionLines)
			{
				const Pulse time = currentPulse + lineIdx * oneLinePulse;
				if (key == "t")
				{
					if (chartData.beat.bpmChanges.empty()) [[unlikely]]
					{
						// In rare cases where BPM is not specified on the chart metadata
						InsertBPMChange(chartData.beat.bpmChanges, 0, value);
					}
					else
					{
						InsertBPMChange(chartData.beat.bpmChanges, time, value);
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

						// Insert cam.tilt_assign
						if (s_tiltTypeScaleTable.contains(value))
						{
							chartData.camera.tilt.scale.insert_or_assign(time, s_tiltTypeScaleTable.at(value));
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
							chartData.note.btLanes[laneIdx].emplace(time, 0);
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
							chartData.note.fxLanes[laneIdx].emplace(time, 0);
							break;
						case '0': // Empty
							preparedLongNoteRef.publishLongFXNote();
							break;
						default:  // Long FX note
							if (!preparedLongNoteRef.prepared())
							{
								if (buf[j] == '1')
								{
									const std::string audioEffectStr = currentMeasureFXAudioEffectStrs[laneIdx].contains(i) ? currentMeasureFXAudioEffectStrs[laneIdx].at(i) : "";
									const std::string audioEffectParamStr = currentMeasureFXAudioEffectParamStrs[laneIdx].contains(i) ? currentMeasureFXAudioEffectParamStrs[laneIdx].at(i) : ""; // Note: Normally this is not used here because it's for legacy long FX chars
									preparedLongNoteRef.prepare(time, audioEffectStr, audioEffectParamStr);
								}
								else
								{
									// Legacy long FX chars
									const std::string audioEffectStr(KSHLegacyFXCharToKSHAudioEffectStr(buf[j]));
									const std::string audioEffectParamStr = currentMeasureFXAudioEffectParamStrs[laneIdx].contains(i) ? currentMeasureFXAudioEffectParamStrs[laneIdx].at(i) : "";
									preparedLongNoteRef.prepare(time, audioEffectStr, audioEffectParamStr);
								}
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
								}
							}
							break;
						}
					}
					else if (currentBlock == kBlockIdxLaser && laneIdx == kNumLaserLanes) // Lane spin
					{
						// Create a lane spin from string
						const PreparedLaneSpin laneSpin = PreparedLaneSpin::FromKSHSpinStr(buf.substr(j), chartData.beat.resolution);
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
			chartLines.clear();
			optionLines.clear();
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
			currentPulse += chartData.beat.resolution * 4 * currentTimeSig.numerator / currentTimeSig.denominator;
			++currentMeasureIdx;
		}
		else
		{
			// TODO: Insert to chartData.compat.kshUnknown.line
		}
	}

	// Publish the last manual tilt section if exists
	if (preparedManualTilt.prepared())
	{
		preparedManualTilt.publishManualTilt();
	}

	// KSH file must end with the bar line "--", so there can never be a prepared button note here
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

	// Convert FX parameters
	for (auto& [audioEffectName, lanes] : chartData.audio.audioEffects.fx.longInvoke)
	{
		AudioEffectType type = AudioEffectType::kUnspecified;
		if (chartData.audio.audioEffects.fx.def.contains(audioEffectName))
		{
			// User-defined audio effects
			type = chartData.audio.audioEffects.fx.def.at(audioEffectName).type;
		}
		else
		{
			// Preset audio effects
			type = StrToAudioEffectType(audioEffectName);
		}

		assert(type != AudioEffectType::kUnspecified);

		for (auto& lane : lanes)
		{
			for (auto& [y, params] : lane)
			{
				// Convert temporary stored "_param1"/"_param2" values to parameter values for each audio effect type
				if (params.contains("_param1") && params.contains("_param2"))
				{
					const std::int32_t param1 = static_cast<std::int32_t>(std::round(params.at("_param1").value));
					const std::int32_t param2 = static_cast<std::int32_t>(std::round(params.at("_param2").value));

					switch (type)
					{
					case AudioEffectType::kRetrigger:
					case AudioEffectType::kGate:
					case AudioEffectType::kWobble:
						if (param1 > 0)
						{
							params.emplace("wave_length", 1.0 / param1);
							// Comment out as the default value is 1.0
							//params.emplace("wave_length@tempo_sync", 1.0);
						}
						break;
					case AudioEffectType::kPitchShift:
						params.emplace("pitch", static_cast<double>(param1));
						break;
					case AudioEffectType::kBitcrusher:
						params.emplace("reduction", static_cast<double>(param1));
						break;
					case AudioEffectType::kTapestop:
						params.emplace("speed", param1 / 100.0);
						break;
					case AudioEffectType::kEcho:
						params.emplace("wave_length", 1.0 / param1);
						// Comment out as the default value is 1.0
						//params.emplace("wave_length@tempo_sync", 1.0);
						params.emplace("feedback", param2 / 100.0);
						break;
					};
					
					params.erase("_param1");
					params.erase("_param2");
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
