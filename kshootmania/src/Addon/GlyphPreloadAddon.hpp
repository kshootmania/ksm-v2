#pragma once

class GlyphPreloadAddon : public IAddon
{
public:
	static constexpr StringView kAddonName = U"GlyphPreload";

private:
	struct PreloadRequest
	{
		Font font;
		String chars;
	};

	Array<PreloadRequest> m_requests;

	// 次に生成する文字の位置
	std::size_t m_requestIdx = 0;
	std::size_t m_charIdx = 0;

	// フォント毎の要求済みの文字
	HashTable<Font::IDType, HashSet<char32>> m_requestedChars;

	bool m_requestedInThisFrame = false;

public:
	GlyphPreloadAddon() = default;

	virtual ~GlyphPreloadAddon() = default;

	virtual bool update() override;

	/// @brief 文字列に含まれる文字のグリフを事前生成するよう要求(要求した順に生成される)
	static void Request(const Font& font, StringView text);

	/// @brief 未生成の要求をキャンセル
	static void CancelPendingRequests();

	/// @brief 未生成の要求と要求済みの文字をリセット(フォントを登録し直した場合に呼ぶ)
	static void Reset();
};
