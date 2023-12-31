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
		// コメント行を読み飛ばす
		if (line[0] == U';' || line.substrView(0, 2) == U"//")
		{
			continue;
		}

		const std::size_t equalPos = line.indexOf(U'=');
		if (equalPos == String::npos)
		{
			// 行内に'='がない
			continue;
		}

		const StringView key = line.substrView(0, equalPos);
		const StringView value = line.substrView(equalPos + 1);
		m_hashTable[key] = value;
	}
}

void KSMIniData::save(FilePathView path) const
{
	constexpr std::size_t kReserveSize = 4096;

	String ini;
	ini.reserve(kReserveSize);

#ifdef _MSC_VER
	// iniファイル内に登場しなかったキーのset
	const auto keysView = std::views::keys(m_hashTable);
	std::set<StringView> remainingKeys(keysView.begin(), keysView.end()); // 順序を安定にするためにunordered_setではなくsetを使用
#else
	// iniファイル内に登場しなかったキーのset
	// (Apple Clangが現状std::views::keys非対応のためforで回す)
	std::set<StringView> remainingKeys; // 順序を安定にするためにunordered_setではなくsetを使用
	for (const auto& [key, _] : m_hashTable)
	{
		remainingKeys.insert(key);
	}
#endif
	
	// iniファイルの内容を読み込みながら, 設定の行を最新の値で書き換える
	if (FileSystem::Exists(path))
	{
		TextReader reader(path); // TODO: ファイル名にShift-JISが指定された場合の考慮
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
			if (hasValue(key))
			{
				remainingKeys.erase(key);
				line = key + U'=' + getString(key); // Note: この代入によって変数keyの参照先が書き換わるため、ここ以降では変数keyを使用しないよう注意
			}

			ini += line;
			ini += U'\n';
		}
	}

	// 残りの値を末尾に追加
	for (const auto& key : remainingKeys)
	{
		ini += key;
		ini += U'=';
		ini += getString(key);
		ini += U'\n';
	}

	// ファイルへ書き込み
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

bool KSMIniData::getBool(StringView key, bool defaultValue) const
{
	return getInt(key, defaultValue ? 1 : 0) != 0;
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

void KSMIniData::setBool(StringView key, bool value)
{
	setInt(key, value ? 1 : 0);
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
