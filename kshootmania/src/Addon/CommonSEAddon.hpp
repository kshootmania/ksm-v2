#pragma once
#include "ksmaudio/ksmaudio.hpp"

enum class CommonSEType
{
	kTitleEnter,
	kSelEnter,
};

class CommonSEAddon : public IAddon
{
public:
	static constexpr StringView kAddonName = U"CommonSE";

private:
	HashTable<CommonSEType, std::unique_ptr<ksmaudio::Sample>> m_samples;

	HashTable<String, std::unique_ptr<ksmaudio::Sample>> m_fileSamples;

public:
	CommonSEAddon();

	virtual ~CommonSEAddon() = default;

	static void Play(CommonSEType type);

	/// @brief 音声ファイルパスを指定して効果音を再生(初回再生時に読み込んでキャッシュ)
	static void PlayFilePath(const String& filePath, double volume);
};
