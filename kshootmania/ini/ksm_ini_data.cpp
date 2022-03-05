#include "ksm_ini_data.hpp"
#include <ranges>

KSMIniData::KSMIniData(FilePathView path)
{
	load(path);
}

void KSMIniData::load(FilePathView path)
{
	if (!FileSystem::Exists(path))
	{
		Print << U"Warning: Could not find INI file '{}'!"_fmt(path);
	}

	TextReader reader(path);
	String line;
	while (reader.readLine(line))
	{
		// Skip comments
		if (line[0] == U';' || line.substrView(0, 2) == U"//")
		{
			continue;
		}

		const std::size_t equalPos = line.indexOf(U'=');
		if (equalPos == String::npos)
		{
			// The line doesn't have '='
			continue;
		}

		const StringView key = line.substrView(0, equalPos);
		const StringView value = line.substrView(equalPos + 1);
		m_hashTable[key] = value;
	}
}

void KSMIniData::save(FilePathView path) const
{
	constexpr std::size_t kReserveSize{ 4096 };

	String ini;
	ini.reserve(kReserveSize);

	// Set of remaining keys to write unappeared keys
	const auto keysView = std::views::keys(m_hashTable);
	std::set<StringView> remainingKeys(keysView.begin(), keysView.end()); // Use ordered set to make write order stable

	// Read latest content and replace value
	if (FileSystem::Exists(path))
	{
		TextReader reader(path); // TODO: Deal with Shift-JIS
		String line;
		while (reader.readLine(line))
		{
			// Skip comments
			if (line[0] == U';' || line.substrView(0, 2) == U"//")
			{
				ini += line;
				ini += U'\n';
				continue;
			}

			const std::size_t equalPos = line.indexOf(U'=');
			if (equalPos == String::npos)
			{
				// The line doesn't have '='
				ini += line;
				ini += U'\n';
				continue;
			}

			const StringView key = line.substrView(0, equalPos);
			const StringView originalValue = line.substrView(equalPos + 1);
			if (hasValue(key))
			{
				line = key + U'=' + getString(key);
				remainingKeys.erase(key);
			}

			ini += line;
			ini += U'\n';
		}
	}

	// Append remaining key values
	for (const auto& key : remainingKeys)
	{
		ini += key;
		ini += U'=';
		ini += getString(key);
		ini += U'\n';
	}

	// Write
	TextWriter writer(path, TextEncoding::UTF8_NO_BOM);
	if (writer)
	{
		writer.write(ini);
	}
	else
	{
		Print << U"Warning: Could not save INI file '{}'!"_fmt(path);
	}
}

bool KSMIniData::hasValue(StringView key) const
{
	return m_hashTable.contains(key);
}

int32 KSMIniData::getInt(StringView key, int32 defaultValue) const
{
	if (!m_hashTable.contains(key))
	{
		return defaultValue;
	}

	try
	{
		return Parse<int32>(m_hashTable.at(key));
	}
	catch (const ParseError&)
	{
		Print << U"Warning: The field '{}' in INI file is ignored! (The value '{}' is not an integer.)"_fmt(m_hashTable.at(key));
		return defaultValue;
	}
}

double KSMIniData::getDouble(StringView key, double defaultValue) const
{
	if (!m_hashTable.contains(key))
	{
		return defaultValue;
	}

	try
	{
		return Parse<int32>(m_hashTable.at(key));
	}
	catch (const ParseError&)
	{
		Print << U"Warning: The field '{}' in INI file is ignored! (The value '{}' is not a number.)"_fmt(m_hashTable.at(key));
		return defaultValue;
	}
}

StringView KSMIniData::getString(StringView key, StringView defaultValue) const
{
	return m_hashTable.contains(key) ? m_hashTable.at(key) : defaultValue;
}

void KSMIniData::setInt(StringView key, int32 value)
{
	m_hashTable[key] = Format(value);
}

void KSMIniData::setDouble(StringView key, double value)
{
	m_hashTable[key] = Format(value);
}

void KSMIniData::setString(StringView key, StringView value)
{
	m_hashTable[key] = value;
}
