#include "CommonSEAddon.hpp"

CommonSEAddon::CommonSEAddon()
{
	m_samples.emplace(CommonSEType::kTitleEnter, std::make_unique<ksmaudio::Sample>("se/title_enter.wav"));
	m_samples.emplace(CommonSEType::kSelEnter, std::make_unique<ksmaudio::Sample>("se/sel_enter.ogg"));
}

void CommonSEAddon::Play(CommonSEType type)
{
	if (const auto pAddon = Addon::GetAddon<CommonSEAddon>(kAddonName))
	{
		if (const auto it = pAddon->m_samples.find(type); it != pAddon->m_samples.end())
		{
			it->second->play();
		}
	}
}

void CommonSEAddon::PlayFilePath(const String& filePath, double volume)
{
	if (const auto pAddon = Addon::GetAddon<CommonSEAddon>(kAddonName))
	{
		auto it = pAddon->m_fileSamples.find(filePath);
		if (it == pAddon->m_fileSamples.end())
		{
			it = pAddon->m_fileSamples.emplace(filePath, std::make_unique<ksmaudio::Sample>(filePath.toUTF8())).first;
		}
		it->second->play(volume);
	}
}
