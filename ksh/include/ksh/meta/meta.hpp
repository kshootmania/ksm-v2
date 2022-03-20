#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct DifficultyInfo
	{
		std::int8_t idx = 0; // 0-3 in KSH
	};

	inline void to_json(nlohmann::json& j, const DifficultyInfo& difficulty)
	{
		j["idx"] = difficulty.idx;
	}

	struct LegacyMetaInfo
	{
		std::u8string titleImageFilename;
		std::u8string artistImageFilename;

		bool empty() const
		{
			return titleImageFilename.empty() && artistImageFilename.empty();
		}
	};

	inline void to_json(nlohmann::json& j, const LegacyMetaInfo& legacy)
	{
		j = nlohmann::json::object();

		if (!legacy.titleImageFilename.empty())
		{
			j["title_img_filename"] = UnU8(legacy.titleImageFilename);
		}

		if (!legacy.artistImageFilename.empty())
		{
			j["artist_img_filename"] = UnU8(legacy.artistImageFilename);
		}
	}

	struct MetaRoot
	{
		std::u8string title;
		std::u8string titleTranslit;
		std::u8string subtitle;
		std::u8string artist;
		std::u8string artistTranslit;
		std::u8string chartAuthor;
		DifficultyInfo difficulty;
		std::int8_t level = 1;
		std::u8string dispBPM;
		double standardBPM = 0.0;
		std::u8string jacketFilename;
		std::u8string jacketAuthor;
		std::u8string information;
		LegacyMetaInfo legacy;
		std::u8string kshVersion;
	};

	inline void to_json(nlohmann::json& j, const MetaRoot& meta)
	{
		j = {
			{ "title", UnU8(meta.title) },
			{ "artist", UnU8(meta.artist) },
			{ "chart_author", UnU8(meta.chartAuthor) },
			{ "difficulty", meta.difficulty },
			{ "level", meta.level },
			{ "disp_bpm", UnU8(meta.dispBPM) },
		};

		if (!meta.titleTranslit.empty())
		{
			j["title_translit"] = UnU8(meta.titleTranslit);
		}

		if (!meta.subtitle.empty())
		{
			j["subtitle"] = UnU8(meta.subtitle);
		}

		if (!meta.artistTranslit.empty())
		{
			j["artist_translit"] = UnU8(meta.artistTranslit);
		}

		if (meta.standardBPM != 0.0)
		{
			j["std_bpm"] = meta.standardBPM;
		}

		if (!meta.information.empty())
		{
			j["information"] = UnU8(meta.information);
		}

		if (!meta.jacketFilename.empty())
		{
			j["jacket_filename"] = UnU8(meta.jacketFilename);
		}

		if (!meta.jacketAuthor.empty())
		{
			j["jacket_author"] = UnU8(meta.jacketAuthor);
		}

		if (!meta.kshVersion.empty())
		{
			j["ksh_version"] = UnU8(meta.kshVersion);
		}
	}
}
