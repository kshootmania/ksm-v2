#pragma once

#include <map>
#include <string>
#include <cstddef>

#include "ksh/beat_map/time_sig.hpp"

class LineGraph
{
public:
    using Plot = std::pair<double, double>;

private:
    std::map<Measure, Plot> m_plots;

public:
    void insert(Measure measure, double plot);
    void insert(Measure measure, Plot plot);
    void insert(Measure measure, const std::string & plot);

    std::size_t erase(Measure measure);

    Plot at(Measure measure) const
    {
        return m_plots.at(measure);
    }

    std::map<Measure, Plot>::iterator begin()
    {
        return m_plots.begin();
    }

    std::map<Measure, Plot>::const_iterator begin() const
    {
        return m_plots.begin();
    }

    std::map<Measure, Plot>::const_iterator cbegin() const
    {
        return m_plots.cbegin();
    }

    std::map<Measure, Plot>::iterator end()
    {
        return m_plots.end();
    }

    std::map<Measure, Plot>::const_iterator end() const
    {
        return m_plots.end();
    }

    std::map<Measure, Plot>::const_iterator cend() const
    {
        return m_plots.cend();
    }

    std::size_t size() const
    {
        return m_plots.size();
    }

    std::size_t count(Measure measure) const
    {
        return m_plots.count(measure);
    }

    double valueAt(Measure measure) const;

    std::string stringValueAt(Measure measure) const;
};
