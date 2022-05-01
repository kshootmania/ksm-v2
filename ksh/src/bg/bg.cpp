#include "ksh/bg/bg.hpp"

bool ksh::KSHRotationFlags::operator==(const KSHRotationFlags& rhs) const
{
	return tiltAffected == rhs.tiltAffected && spinAffected == rhs.spinAffected;
}

void ksh::to_json(nlohmann::json& j, const KSHRotationFlags& flags)
{
	j = {
		{ "tilt", flags.tiltAffected },
		{ "spin", flags.spinAffected },
	};
}

void ksh::from_json(const nlohmann::json& j, KSHRotationFlags& flags)
{
	j.at("tilt").get_to(flags.tiltAffected);
	j.at("spin").get_to(flags.spinAffected);
}

bool ksh::KSHBGInfo::operator==(const KSHBGInfo& rhs) const
{
	return filename == rhs.filename && rotationFlags == rhs.rotationFlags;
}

void ksh::to_json(nlohmann::json& j, const KSHBGInfo& bg)
{
	j = {
		{ "filename", bg.filename },
	};

	if (bg.rotationFlags != KSHRotationFlags{ true, false })
	{
		j["rotation"] = bg.rotationFlags;
	}
}

bool ksh::KSHLayerInfo::operator==(const KSHLayerInfo& rhs) const
{
	return filename == rhs.filename && durationMs == rhs.durationMs && rotationFlags == rhs.rotationFlags;
}

void ksh::to_json(nlohmann::json& j, const KSHLayerInfo& layer)
{
	j = {
		{ "filename", layer.filename },
	};

	if (layer.durationMs > 0)
	{
		j["duration"] = layer.durationMs;
	}

	if (layer.rotationFlags != KSHRotationFlags{ true, true })
	{
		j["rotation"] = layer.rotationFlags;
	}
}

bool ksh::KSHMovieInfo::empty() const
{
	return filename.empty() && offsetMs == 0;
}

void ksh::to_json(nlohmann::json& j, const KSHMovieInfo& movie)
{
	j = {
		{ "filename", movie.filename },
		{ "offset", movie.offsetMs },
	};
}

void ksh::to_json(nlohmann::json& j, const LegacyBGRoot& legacy)
{
	j = nlohmann::json::object();

	if (legacy.bgInfos[0] == legacy.bgInfos[1])
	{
		j["bg"] = nlohmann::json::array({ legacy.bgInfos[0] });
	}
	else
	{
		j["bg"] = legacy.bgInfos;
	}

	if (legacy.layerInfos[0] == legacy.layerInfos[1])
	{
		j["layer"] = nlohmann::json::array({ legacy.layerInfos[0] });
	}
	else
	{
		j["layer"] = legacy.layerInfos;
	}

	if (!legacy.movieInfos.filename.empty() || legacy.movieInfos.offsetMs != 0)
	{
		j["movie"] = legacy.movieInfos;
	}
}

bool ksh::LegacyBGRoot::empty() const
{
	return bgInfos.empty() && layerInfos.empty() && movieInfos.empty();
}

void ksh::to_json(nlohmann::json& j, const BGRoot& bg)
{
	j = nlohmann::json::object();

	if (!bg.legacy.empty())
	{
		j["legacy"] = bg.legacy;
	}
}