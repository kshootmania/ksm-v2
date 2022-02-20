#include "ksh/chart_object/line_graph.hpp"
#include <ios>
#include <sstream>

void LineGraph::insert(Measure measure, double plot)
{
    if (m_plots.count(measure))
    {
        m_plots[measure].second = plot;
    }
    else
    {
        insert(measure, std::make_pair(plot, plot));
    }
}

void LineGraph::insert(Measure measure, Plot plot)
{
    m_plots.insert(std::make_pair(measure, plot));
}

void LineGraph::insert(Measure measure, const std::string & plot)
{
    // TODO: catch exceptions from std::stod()
    const std::size_t semicolonIdx = plot.find(';');
    if (semicolonIdx == std::string::npos)
    {
        double value = std::stod(plot);
        insert(measure, std::make_pair(value, value));
    }
    else
    {
        insert(measure, std::make_pair(
            std::stod(plot.substr(0, semicolonIdx)),
            std::stod(plot.substr(semicolonIdx + 1))));
    }
}

std::size_t LineGraph::erase(Measure measure)
{
    return m_plots.erase(measure);
}

double LineGraph::valueAt(Measure measure) const
{
    if (m_plots.empty())
    {
        return 0.0;
    }

    const auto secondItr = m_plots.upper_bound(measure);
    if (secondItr == m_plots.begin())
    {
        // Before the first plot
        return (*secondItr).second.first;
    }

    const auto firstItr = std::prev(secondItr);
    const double firstValue = (*firstItr).second.second;

    if (secondItr == m_plots.end())
    {
        // After the last plot
        return firstValue;
    }
    else
    {
        const double secondValue = (*secondItr).second.first;
        const Measure firstMeasure = (*firstItr).first;
        const Measure secondMeasure = (*secondItr).first;
        return firstValue + (secondValue - firstValue) * (measure - firstMeasure) / (secondMeasure - firstMeasure);
    }
}

std::string LineGraph::stringValueAt(Measure measure) const
{
    std::stringstream ss;
    ss << std::fixed; // Avoid scientific notation
    if (m_plots.count(measure))
    {
        const Plot plot = m_plots.at(measure);
        if (plot.first == plot.second)
        {
            ss << plot.first;
        }
        else
        {
            ss << plot.first << ';' << plot.second;
        }
    }
    else
    {
        ss << valueAt(measure);
    }
    std::string str = ss.str();
    while (!str.empty() && *str.rbegin() == '0') str.pop_back();
    if (!str.empty() && *str.rbegin() == '.') str.pop_back();
    return str;
}
