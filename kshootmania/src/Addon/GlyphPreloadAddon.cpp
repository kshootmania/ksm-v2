#include "GlyphPreloadAddon.hpp"

namespace
{
	// 1フレームでグリフの生成に使う時間の上限
	constexpr Duration kPreloadTimeLimitPerFrame = 0.001s;
}

bool GlyphPreloadAddon::update()
{
	if (m_requestedInThisFrame)
	{
		// 要求があったフレームは表示の更新で負荷が高いため生成しない
		m_requestedInThisFrame = false;
		return true;
	}

	if (m_requests.empty())
	{
		return true;
	}

	const Stopwatch stopwatch{ StartImmediately::Yes };
	while (m_requestIdx < m_requests.size())
	{
		const PreloadRequest& request = m_requests[m_requestIdx];
		if (m_charIdx >= request.chars.size())
		{
			++m_requestIdx;
			m_charIdx = 0;
			continue;
		}

		request.font.preload(StringView{ request.chars }.substr(m_charIdx, 1));
		++m_charIdx;

		if (stopwatch.elapsed() >= kPreloadTimeLimitPerFrame)
		{
			break;
		}
	}

	if (m_requestIdx >= m_requests.size())
	{
		m_requests.clear();
		m_requestIdx = 0;
		m_charIdx = 0;
	}

	return true;
}

void GlyphPreloadAddon::Request(const Font& font, StringView text)
{
	if (const auto pAddon = Addon::GetAddon<GlyphPreloadAddon>(kAddonName))
	{
		pAddon->m_requestedInThisFrame = true;

		auto& requestedChars = pAddon->m_requestedChars[font.id()];
		String chars;
		for (const char32 ch : text)
		{
			if (requestedChars.insert(ch).second)
			{
				chars.push_back(ch);
			}
		}
		if (!chars.isEmpty())
		{
			pAddon->m_requests.push_back(PreloadRequest{ .font = font, .chars = std::move(chars) });
		}
	}
}

void GlyphPreloadAddon::CancelPendingRequests()
{
	if (const auto pAddon = Addon::GetAddon<GlyphPreloadAddon>(kAddonName))
	{
		// 未生成の文字は、次に要求されたときに生成されるよう要求済みの文字から外す
		for (std::size_t requestIdx = pAddon->m_requestIdx; requestIdx < pAddon->m_requests.size(); ++requestIdx)
		{
			const PreloadRequest& request = pAddon->m_requests[requestIdx];
			auto& requestedChars = pAddon->m_requestedChars[request.font.id()];
			const std::size_t beginCharIdx = (requestIdx == pAddon->m_requestIdx) ? pAddon->m_charIdx : 0;
			for (std::size_t charIdx = beginCharIdx; charIdx < request.chars.size(); ++charIdx)
			{
				requestedChars.erase(request.chars[charIdx]);
			}
		}

		pAddon->m_requests.clear();
		pAddon->m_requestIdx = 0;
		pAddon->m_charIdx = 0;
	}
}

void GlyphPreloadAddon::Reset()
{
	if (const auto pAddon = Addon::GetAddon<GlyphPreloadAddon>(kAddonName))
	{
		pAddon->m_requests.clear();
		pAddon->m_requestIdx = 0;
		pAddon->m_charIdx = 0;
		pAddon->m_requestedChars.clear();
	}
}
