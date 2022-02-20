#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <fstream>

namespace ksh
{

    // Chart (header)
    class Chart
    {
    private:
        bool m_isUTF8;

    protected:
        const std::string m_filename;
        const std::string m_fileDirectoryPath;
        std::unique_ptr<std::ifstream> m_ifs; // uses unique_ptr to make Chart objects move constructible
        int m_difficultyIdx;
        Chart(std::string_view filename, bool keepFileStreamOpen);

    public:
        // Chart meta data
        std::unordered_map<std::string, std::string> metaData;

        explicit Chart(std::string_view filename);

        virtual ~Chart() = default;

        std::string toString() const;

        bool isKshVersionNewerThanOrEqualTo(int version) const;

        int kshVersionInt() const;

        int difficultyIdx() const
        {
            return m_difficultyIdx;
        }

        bool isUTF8() const
        {
            return m_isUTF8;
        }
    };

}
