#include "kson/bg/bg_info.hpp"

void kson::to_json(nlohmann::json& j, const KSHLayerRotationInfo& flags)
{
	j = {
		{ "tilt", flags.tilt },
		{ "spin", flags.spin },
	};
}

void kson::from_json(const nlohmann::json& j, KSHLayerRotationInfo& flags)
{
	j.at("tilt").get_to(flags.tilt);
	j.at("spin").get_to(flags.spin);
}

bool kson::KSHBGInfo::operator==(const KSHBGInfo& rhs) const
{
	return filename == rhs.filename;
}

void kson::to_json(nlohmann::json& j, const KSHBGInfo& bg)
{
	j = {
		{ "filename", bg.filename },
	};
}

bool kson::KSHLayerInfo::empty() const
{
	return filename.empty() && duration == 0 && rotation.tilt == true && rotation.spin == true;
}

void kson::to_json(nlohmann::json& j, const KSHLayerInfo& layer)
{
	j = {
		{ "filename", layer.filename },
	};

	if (layer.duration > 0)
	{
		j["duration"] = layer.duration;
	}

	if (layer.rotation.tilt != true || layer.rotation.spin != true) // default: { tilt:true, spin:true }
	{
		j["rotation"] = layer.rotation;
	}
}

bool kson::KSHMovieInfo::empty() const
{
	return filename.empty() && offset == 0;
}

void kson::to_json(nlohmann::json& j, const KSHMovieInfo& movie)
{
	j = {
		{ "filename", movie.filename },
		{ "offset", movie.offset },
	};
}

void kson::to_json(nlohmann::json& j, const LegacyBGInfo& legacy)
{
	j = nlohmann::json::object();

	if (legacy.bg[0] == legacy.bg[1])
	{
		j["bg"] = nlohmann::json::array({ legacy.bg[0] });
	}
	else
	{
		j["bg"] = legacy.bg;
	}

	if (!legacy.layer.empty())
	{
		j["layer"] = legacy.layer;
	}

	if (!legacy.movie.filename.empty() || legacy.movie.offset != 0)
	{
		j["movie"] = legacy.movie;
	}
}

bool kson::LegacyBGInfo::empty() const
{
	return nlohmann::json(*this).empty();
}

void kson::to_json(nlohmann::json& j, const BGInfo& bg)
{
	j = nlohmann::json::object();

	if (!bg.legacy.empty())
	{
		j["legacy"] = bg.legacy;
	}
}
