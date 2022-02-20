#pragma once

#include <string>
#include <cassert>

#include "ksh/beat_map/time_sig.hpp"

struct LaneSpin
{
    enum class Type
    {
        NoSpin,
        Normal,
        Half,
        Swing,
    };
    Type type;

    enum class Direction
    {
        Unspecified,
        Left,
        Right,
    };
    Direction direction;

    Measure length;

    int swingAmplitude;

    std::size_t swingFrequency;

    int swingDecayOrder;

    LaneSpin() : type(Type::NoSpin), direction(Direction::Unspecified), length(0), swingAmplitude(0), swingFrequency(0), swingDecayOrder(0) {}

    LaneSpin(Type type, Direction direction, Measure length) : type(type), direction(direction), length(length), swingAmplitude(0), swingFrequency(0), swingDecayOrder(0) {}

    LaneSpin(Type type, Direction direction, Measure length, int swingAmplitude, std::size_t swingFrequency, int swingDecayOrder)
        : type(type)
        , direction(direction)
        , length(length)
        , swingAmplitude(swingAmplitude)
        , swingFrequency(swingFrequency)
        , swingDecayOrder(swingDecayOrder)
    {
        assert(type == Type::Swing);
    }

    explicit LaneSpin(const std::string & strFromKsh); // From .ksh spin string (example: "@(192")

    bool isValid() const
    {
        return type != LaneSpin::Type::NoSpin && direction != LaneSpin::Direction::Unspecified;
    }

    std::string toString() const; // To .ksh spin string (example: "@(192")
};
