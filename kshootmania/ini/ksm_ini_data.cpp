#include "ksm_ini_data.hpp"

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
	// TODO: implement
	Print << U"Warning: KSMIniData::save() is not implemented!";
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
