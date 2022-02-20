#include "ksh/chart_object/lane_spin.hpp"

#include <sstream>
#include <array>
#include <tuple>
#include "ksh/beat_map/time_sig.hpp"

Measure kshLengthToMeasure(const std::string & str)
{
    // TODO: catch the exception from stoi
    return std::stoll(str) * UNIT_MEASURE / 192;
}

std::string measureToKshLength(Measure measure)
{
    return std::to_string(measure * 192 / UNIT_MEASURE);
}

std::tuple<Measure, int, std::size_t, int> splitSwingParams(const std::string & paramStr)
{
    std::array<std::string, 4> params{
        "192", "250", "3", "2"
    };

    std::stringstream ss(paramStr);
    std::string s;
    int i = 0;
    while (i < 4 && std::getline(ss, s, ';'))
    {
        params[i] = s;
        ++i;
    }

    return std::make_tuple(
        kshLengthToMeasure(params[0]),
        std::stoi(params[1]),
        static_cast<std::size_t>(std::stoll(params[2])),
        std::stoi(params[3]));
}

LaneSpin::LaneSpin(const std::string & strFromKsh)
{
    // A .ksh spin string should have at least 3 chars
    if (strFromKsh.length() < 3)
    {
        type = Type::NoSpin;
        direction = Direction::Unspecified;
        length = 0;
        return;
    }

    // Specify the spin type
    if (strFromKsh[0] == '@')
    {
        switch (strFromKsh[1])
        {
        case '(':
            type = Type::Normal;
            direction = Direction::Left;
            break;

        case ')':
            type = Type::Normal;
            direction = Direction::Right;
            break;

        case '<':
            type = Type::Half;
            direction = Direction::Left;
            break;

        case '>':
            type = Type::Half;
            direction = Direction::Right;
            break;

        default:
            type = Type::NoSpin;
            direction = Direction::Unspecified;
            break;
        }
    }
    else if (strFromKsh[0] == 'S')
    {
        switch (strFromKsh[1])
        {
        case '<':
            type = Type::Swing;
            direction = Direction::Left;
            break;

        case '>':
            type = Type::Swing;
            direction = Direction::Right;
            break;

        default:
            type = Type::NoSpin;
            direction = Direction::Unspecified;
            break;
        }
    }
    else
    {
        type = Type::NoSpin;
        direction = Direction::Unspecified;
    }

    // Specify the spin length
    if (type == Type::NoSpin || direction == Direction::Unspecified)
    {
        length = 0;
    }
    else if (type == Type::Swing)
    {
        std::tie(length, swingAmplitude, swingFrequency, swingDecayOrder) = splitSwingParams(strFromKsh.substr(2));
    }
    else
    {
        length = kshLengthToMeasure(strFromKsh.substr(2));
    }
}

// TODO: Separate these ksh-specific methods
std::string LaneSpin::toString() const
{
    if (direction == Direction::Unspecified)
    {
        return "";
    }
    else
    {
        switch (type)
        {
        case Type::Normal:
            return std::string("@") + ((direction == Direction::Left) ? "(" : ")") + measureToKshLength(length);

        case Type::Half:
            return std::string("@") + ((direction == Direction::Left) ? "<" : ">") + measureToKshLength(length);

        case Type::Swing:
            return
                std::string("S") +
                ((direction == Direction::Left) ? "<" : ">") +
                measureToKshLength(length) + ";" +
                std::to_string(swingAmplitude) + ";" +
                std::to_string(swingFrequency) + ";" +
                std::to_string(swingDecayOrder);

        default:
            return "";
        }
    }
}
