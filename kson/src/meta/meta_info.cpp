#include "kson/meta/meta_info.hpp"

void kson::to_json(nlohmann::json& j, const DifficultyInfo& difficulty)
{
	j["idx"] = difficulty.idx;
}

void kson::to_json(nlohmann::json& j, const MetaInfo& meta)
{
	j = {
		{ "title", meta.title },
		{ "artist", meta.artist },
		{ "chart_author", meta.chartAuthor },
		{ "difficulty", meta.difficulty },
		{ "level", meta.level },
		{ "disp_bpm", meta.dispBPM },
	};

	if (!meta.titleImgFilename.empty())
	{
		j["title_img_filename"] = meta.titleImgFilename;
	}

	if (!meta.artistImgFilename.empty())
	{
		j["artist_img_filename"] = meta.artistImgFilename;
	}

	if (meta.stdBPM != 0.0)
	{
		j["std_bpm"] = meta.stdBPM;
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
}
