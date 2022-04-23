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
		std::string titleImageFilename;
		std::string artistImageFilename;

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
			j["title_img_filename"] = legacy.titleImageFilename;
		}

		if (!legacy.artistImageFilename.empty())
		{
			j["artist_img_filename"] = legacy.artistImageFilename;
		}
	}

	struct MetaRoot
	{
		std::string title;
		std::string titleTranslit;
		std::string subtitle;
		std::string artist;
		std::string artistTranslit;
		std::string chartAuthor;
		DifficultyInfo difficulty;
		std::int8_t level = 1;
		std::string dispBPM;
		double standardBPM = 0.0;
		std::string jacketFilename;
		std::string jacketAuthor;
		std::string information;
		LegacyMetaInfo legacy;
		std::string kshVersion;
	};

	inline void to_json(nlohmann::json& j, const MetaRoot& meta)
	{
		j = {
			{ "title", meta.title },
			{ "artist", meta.artist },
			{ "chart_author", meta.chartAuthor },
			{ "difficulty", meta.difficulty },
			{ "level", meta.level },
			{ "disp_bpm", meta.dispBPM },
		};

		if (!meta.titleTranslit.empty())
		{
			j["title_translit"] = meta.titleTranslit;
		}

		if (!meta.subtitle.empty())
		{
			j["subtitle"] = meta.subtitle;
		}

		if (!meta.artistTranslit.empty())
		{
			j["artist_translit"] = meta.artistTranslit;
		}

		if (meta.standardBPM != 0.0)
		{
			j["std_bpm"] = meta.standardBPM;
		}

		if (!meta.information.empty())
		{
			j["information"] = meta.information;
		}

		if (!meta.jacketFilename.empty())
		{
			j["jacket_filename"] = meta.jacketFilename;
		}

		if (!meta.jacketAuthor.empty())
		{
			j["jacket_author"] = meta.jacketAuthor;
		}

		if (!meta.kshVersion.empty())
		{
			j["ksh_version"] = meta.kshVersion;
		}
	}
}
