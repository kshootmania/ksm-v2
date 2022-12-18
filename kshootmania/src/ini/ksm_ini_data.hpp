#pragma once

// Note: s3d::INIData is not used because KSMv1's ini file is not well-formed.
class KSMIniData
{
private:
	HashTable<String, String> m_hashTable;

public:
	KSMIniData() = default;

	explicit KSMIniData(FilePathView path);

	void load(FilePathView path);

	void save(FilePathView path) const;

	bool hasValue(StringView key) const;

	bool getBool(StringView key, bool defaultValue = false) const;

	int32 getInt(StringView key, int32 defaultValue = 0) const;

	double getDouble(StringView key, double defaultValue = 0.0) const;

	StringView getString(StringView key, StringView defaultValue = U"") const;

	void setBool(StringView key, bool value);

	void setInt(StringView key, int32 value);

	void setDouble(StringView key, double value);

	void setString(StringView key, StringView value);
};
