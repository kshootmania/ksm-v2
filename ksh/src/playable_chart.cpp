#include "ksh/playable_chart.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cassert>

#include "ksh/note_builder.hpp"

namespace ksh
{

    constexpr unsigned char OPTION_SEPARATOR = '=';
    constexpr unsigned char BLOCK_SEPARATOR = '|';
    const std::string MEASURE_SEPARATOR = "--";

    constexpr std::size_t BLOCK_BT = 0;
    constexpr std::size_t BLOCK_FX = 1;
    constexpr std::size_t BLOCK_LASER = 2;

    // Maximum value of zoom
    constexpr double ZOOM_ABS_MAX_LEGACY = 300.0; // ver <  1.67
    constexpr double ZOOM_ABS_MAX = 65535.0;      // ver >= 1.67

    // Maximum number of characters of the zoom value
    constexpr std::size_t ZOOM_MAX_CHAR_LEGACY = 4;          // ver <  1.67
    constexpr std::size_t ZOOM_MAX_CHAR = std::string::npos; // ver >= 1.67

    // Maximum value of center_split / manual tilt
    constexpr double CENTER_SPLIT_ABS_MAX = 65535.0;
    constexpr double MANUAL_TILT_ABS_MAX = 1000.0;

    bool isChartLine(const std::string & line)
    {
        return line.find(BLOCK_SEPARATOR) != std::string::npos;
    }

    bool isOptionLine(const std::string & line)
    {
        return line.find(OPTION_SEPARATOR) != std::string::npos;
    }

    bool isBarLine(const std::string & line)
    {
        return line == MEASURE_SEPARATOR;
    }

    std::pair<std::string, std::string> splitOptionLine(const std::string & optionLine)
    {
        std::size_t equalIdx = optionLine.find_first_of(OPTION_SEPARATOR);

        // Option line should have "="
        assert(equalIdx != std::string::npos);

        return std::pair<std::string, std::string>(
            optionLine.substr(0, equalIdx),
            optionLine.substr(equalIdx + 1)
        );
    }

    constexpr bool halvesCombo(double tempo)
    {
        return tempo >= 256.0;
    }

    std::string kshLegacyFXCharToAudioEffect(unsigned char c)
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

    bool isManualTiltValue(const std::string & value)
    {
        return !value.empty() && ((value[0] >= '0' && value[0] <= '9') || value[0] == '-');
    }

    bool PlayableChart::insertTempoChange(std::map<Measure, double> & tempoChanges, Measure y, const std::string & value)
    {
        if (tempoChanges.count(y))
        {
            tempoChanges[y] = std::stod(value);
            return true;
        }
        else if (value.find('-') == std::string::npos)
        {
            tempoChanges.emplace(y, std::stod(value));
            return true;
        }
        else
        {
            return false;
        }
    }

    TimeSig parseTimeSig(const std::string & str)
    {
        std::size_t slashIdx = str.find('/');
        assert(slashIdx != std::string::npos);

        return TimeSig{
            static_cast<uint32_t>(std::stoi(str.substr(0, slashIdx))),
            static_cast<uint32_t>(std::stoi(str.substr(slashIdx + 1)))
        };
    }

    PlayableChart::PlayableChart(std::string_view filename, bool isEditor)
        : Chart(filename, true)
        , m_btLanes(4)
        , m_fxLanes(2)
        , m_laserLanes(2)
    {
        // TODO: Catch exceptions from std::stod()

        // For backward compatibility of zoom_top/zoom_bottom/zoom_side
        const double zoomAbsMax = isKshVersionNewerThanOrEqualTo(167) ? ZOOM_ABS_MAX : ZOOM_ABS_MAX_LEGACY;
        const std::size_t zoomMaxChar = isKshVersionNewerThanOrEqualTo(167) ? ZOOM_MAX_CHAR : ZOOM_MAX_CHAR_LEGACY;

        std::map<Measure, double> tempoChanges;
        std::map<int, TimeSig> timeSigChanges;

        // Note builders for note insertion to lanes
        std::vector<BTNoteBuilder> btNoteBuilders;
        for (auto && lane : m_btLanes)
        {
            btNoteBuilders.emplace_back(lane);
        }
        std::vector<FXNoteBuilder> fxNoteBuilders;
        for (auto && lane : m_fxLanes)
        {
            fxNoteBuilders.emplace_back(lane);
        }
        std::vector<LaserNoteBuilder> laserNoteBuilders;
        for (auto && lane : m_laserLanes)
        {
            laserNoteBuilders.emplace_back(lane);
        }

        // FX audio effect string ("fx-l=" or "fx-r=" in .ksh)
        std::vector<std::string> currentFXAudioEffectStrs(m_fxLanes.size());

        // FX audio effect parameters ("fx-l_param1=" or "fx-r_param1=" in .ksh; currently no "param2")
        std::vector<std::string> currentFXAudioEffectParamStrs(m_fxLanes.size());

        // Insert the first tempo change
        double currentTempo = 120.0;
        if (metaData.count("t"))
        {
            if (insertTempoChange(tempoChanges, 0, metaData.at("t")))
            {
                currentTempo = tempoChanges.at(0);
            }
        }

        // Insert the first time signature change
        uint32_t currentNumerator = 4;
        uint32_t currentDenominator = 4;
        if (metaData.count("beat"))
        {
            TimeSig timeSig = parseTimeSig(metaData.at("beat"));
            timeSigChanges.emplace(
                0,
                timeSig
            );
            currentNumerator = timeSig.numerator;
            currentDenominator = timeSig.denominator;
        }
        else
        {
            timeSigChanges[0] = { 4, 4 };
        }

        // Buffers
        // (needed because actual addition cannot come before the measure value calculation)
        std::vector<std::string> chartLines;
        using OptionLine = std::pair<std::size_t, std::pair<std::string, std::string>>; // first = line index of chart lines
        std::vector<OptionLine> optionLines;

        Measure currentMeasure = 0;
        int measureCount = 0;

        // Read chart body
        // Expect m_ifs to start from the next of the first bar line ("--")
        std::string line;
        while (std::getline(*m_ifs, line, '\n'))
        {
            // Eliminate CR
            if(!line.empty() && *line.crbegin() == '\r') {
                line.pop_back();
            }

            // Skip comments
            if (line[0] == ';' || line.substr(0, 2) == "//")
            {
                continue;
            }

            // TODO: Read user-defined audio effects
            if (line[0] == '#')
            {
                continue;
            }

            if (isChartLine(line))
            {
                chartLines.push_back(line);
            }
            else if (isOptionLine(line))
            {
                auto [ key, value ] = splitOptionLine(line);
                if (key == "t")
                {
                    if (value.find('-') == std::string::npos)
                    {
                        currentTempo = std::stod(value);
                    }
                    optionLines.emplace_back(chartLines.size(), std::make_pair(key, value));
                }
                else if (key == "beat")
                {
                    TimeSig timeSig = parseTimeSig(value);
                    timeSigChanges.emplace(
                        measureCount,
                        timeSig
                    );
                    currentNumerator = timeSig.numerator;
                    currentDenominator = timeSig.denominator;
                }
                else if (key == "fx-l")
                {
                    currentFXAudioEffectStrs[0] = value;
                }
                else if (key == "fx-r")
                {
                    currentFXAudioEffectStrs[1] = value;
                }
                else if (key == "fx-l_param1")
                {
                    currentFXAudioEffectParamStrs[0] = value;
                }
                else if (key == "fx-r_param1")
                {
                    currentFXAudioEffectParamStrs[1] = value;
                }
                else
                {
                    optionLines.emplace_back(chartLines.size(), std::make_pair(key, value));
                }
            }
            else if (isBarLine(line))
            {
                std::size_t resolution = chartLines.size();
                Measure lineYDiff = UNIT_MEASURE * currentNumerator / currentDenominator / resolution;

                // Add options that require their position
                for (const auto & [ lineIdx, option ] : optionLines)
                {
                    const auto & [ key, value ] = option;
                    Measure y = currentMeasure + lineYDiff * lineIdx;
                    if (key == "t")
                    {
                        insertTempoChange(tempoChanges, y, value);
                    }
                    else if (key == "zoom_top")
                    {
                        double dValue = std::stod(value.substr(0, zoomMaxChar));
                        if (std::abs(dValue) <= zoomAbsMax || (!isKshVersionNewerThanOrEqualTo(167) && m_zoomTop.count(y) > 0))
                        {
                            m_zoomTop.insert(y, dValue);
                        }
                    }
                    else if (key == "zoom_bottom")
                    {
                        double dValue = std::stod(value.substr(0, zoomMaxChar));
                        if (std::abs(dValue) <= zoomAbsMax || (!isKshVersionNewerThanOrEqualTo(167) && m_zoomBottom.count(y) > 0))
                        {
                            m_zoomBottom.insert(y, dValue);
                        }
                    }
                    else if (key == "zoom_side")
                    {
                        double dValue = std::stod(value.substr(0, zoomMaxChar));
                        if (std::abs(dValue) <= zoomAbsMax || (!isKshVersionNewerThanOrEqualTo(167) && m_zoomSide.count(y) > 0))
                        {
                            m_zoomSide.insert(y, dValue);
                        }
                    }
                    else if (key == "center_split")
                    {
                        double dValue = std::stod(value);
                        if (std::abs(dValue) <= CENTER_SPLIT_ABS_MAX)
                        {
                            m_centerSplit.insert(y, dValue);
                        }
                    }
                    else if (key == "tilt")
                    {
                        if (isManualTiltValue(value))
                        {
                            double dValue = std::stod(value);
                            if (std::abs(dValue) <= MANUAL_TILT_ABS_MAX)
                            {
                                m_manualTilt.insert(y, std::stod(value));
                                m_positionalOptions[key][y] = "manual";
                            }
                        }
                        else
                        {
                            if (!m_positionalOptions[key].empty() && (*m_positionalOptions[key].rbegin()).second == "manual")
                            {
                                // Insert previous value to keep last value until non-manual tilt type is set
                                m_manualTilt.insert(y, m_manualTilt.valueAt(y));
                            }
                            m_positionalOptions[key][y] = value;
                        }
                    }
                    else
                    {
                        m_positionalOptions[key][y] = value;
                    }
                }

                // Add notes
                for (std::size_t i = 0; i < resolution; ++i)
                {
                    const std::string buf = chartLines.at(i);
                    std::size_t currentBlock = 0;
                    std::size_t laneCount = 0;

                    const Measure y = currentMeasure + lineYDiff * i;

                    for (std::size_t j = 0; j < buf.size(); ++j)
                    {
                        if (buf[j] == BLOCK_SEPARATOR)
                        {
                            ++currentBlock;
                            laneCount = 0;
                            continue;
                        }

                        if (currentBlock == BLOCK_BT) // BT notes
                        {
                            assert(laneCount < btNoteBuilders.size());
                            switch (buf[j])
                            {
                            case '2': // Long BT note
                                btNoteBuilders[laneCount].prepareNote(y, halvesCombo(currentTempo));
                                btNoteBuilders[laneCount].extendPreparedNoteLength(lineYDiff);
                                break;
                            case '1': // Chip BT note
                                m_btLanes[laneCount].emplace(y, BTNote(0));
                                break;
                            default:  // Empty
                                btNoteBuilders[laneCount].addPreparedNote();
                            }
                        }
                        else if (currentBlock == BLOCK_FX) // FX notes
                        {
                            assert(laneCount < fxNoteBuilders.size());
                            switch (buf[j])
                            {
                            case '2': // Chip FX note
                                m_fxLanes[laneCount].emplace(y, FXNote(0));
                                break;
                            case '0': // Empty
                                fxNoteBuilders[laneCount].addPreparedNote();
                                break;
                            default:  // Long FX note
                                const std::string audioEffectStr = (buf[j] == '1') ? currentFXAudioEffectStrs[laneCount] : kshLegacyFXCharToAudioEffect(buf[j]);
                                fxNoteBuilders[laneCount].prepareNote(y, halvesCombo(currentTempo), audioEffectStr, currentFXAudioEffectParamStrs[laneCount], isEditor);
                                fxNoteBuilders[laneCount].extendPreparedNoteLength(lineYDiff);
                            }
                        }
                        else if (currentBlock == BLOCK_LASER && laneCount < 2) // Laser notes
                        {
                            assert(laneCount < laserNoteBuilders.size());
                            switch (buf[j])
                            {
                            case '-': // Empty
                                laserNoteBuilders[laneCount].resetPreparedNote();
                                break;
                            case ':': // Connection
                                laserNoteBuilders[laneCount].extendPreparedNoteLength(lineYDiff);
                                break;
                            default:
                                const int laserX = LaserNote::charToLaserX(buf[j]);
                                if (laserX >= 0)
                                {
                                    laserNoteBuilders[laneCount].addPreparedNote(laserX);
                                    laserNoteBuilders[laneCount].prepareNote(y, halvesCombo(currentTempo), laserX);
                                    laserNoteBuilders[laneCount].extendPreparedNoteLength(lineYDiff);
                                }
                            }
                        }
                        else if (currentBlock == BLOCK_LASER && laneCount == 2) // Lane spin
                        {
                            // Create a lane spin from string
                            const LaneSpin laneSpin(buf.substr(j));
                            if (laneSpin.isValid())
                            {
                                // Assign to the laser note builder if valid
                                for (std::size_t k = 0; k < laserNoteBuilders.size(); ++k)
                                {
                                    laserNoteBuilders[k].prepareLaneSpin(laneSpin);
                                }
                            }
                        }
                        ++laneCount;
                    }
                }
                chartLines.clear();
                optionLines.clear();
                for (auto && str : currentFXAudioEffectStrs)
                {
                    str.clear();
                }
                for (auto && str : currentFXAudioEffectParamStrs)
                {
                    str.clear();
                }
                currentMeasure += UNIT_MEASURE * currentNumerator / currentDenominator;
                ++measureCount;
            }
        }

        m_ifs->close();

        m_beatMap = std::make_unique<BeatMap>(tempoChanges, timeSigChanges);
    }

    std::size_t PlayableChart::comboCount() const
    {
        std::size_t sum = 0;
        for (auto && lane : m_btLanes)
        {
            for (auto && pair : lane)
            {
                sum += pair.second.comboCount();
            }
        }
        for (auto && lane : m_fxLanes)
        {
            for (auto && pair : lane)
            {
                sum += pair.second.comboCount();
            }
        }
        for (auto && lane : m_laserLanes)
        {
            for (auto && pair : lane)
            {
                sum += pair.second.comboCount();
            }
        }
        return sum;
    }

}
