#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
	struct KSHRotationFlags
	{
		bool tiltAffected = false;
		bool spinAffected = false;

		bool operator==(const KSHRotationFlags& rhs) const
		{
			return tiltAffected == rhs.tiltAffected && spinAffected == rhs.spinAffected;
		}
	};

	inline void to_json(nlohmann::json& j, const KSHRotationFlags& flags)
	{
		j = {
			{ "tilt", flags.tiltAffected },
			{ "spin", flags.spinAffected },
		};
	}

	inline void from_json(const nlohmann::json& j, KSHRotationFlags& flags)
	{
		j.at("tilt").get_to(flags.tiltAffected);
		j.at("spin").get_to(flags.spinAffected);
	}

	struct KSHBGInfo
	{
		std::string filename; // UTF-8 guaranteed
		KSHRotationFlags rotationFlags = { true, false };

		bool operator==(const KSHBGInfo& rhs) const
		{
			return filename == rhs.filename && rotationFlags == rhs.rotationFlags;
		}
	};

	inline void to_json(nlohmann::json& j, const KSHBGInfo& bg)
	{
		j = {
			{ "filename", bg.filename },
		};

		if (bg.rotationFlags != KSHRotationFlags{ true, false })
		{
			j["rotation"] = bg.rotationFlags;
		}
	}

	struct KSHLayerInfo
	{
		std::string filename; // UTF-8 guaranteed
		std::int64_t durationMs = 0;
		KSHRotationFlags rotationFlags = { true, true };

		bool operator==(const KSHLayerInfo& rhs) const
		{
			return filename == rhs.filename && durationMs == rhs.durationMs && rotationFlags == rhs.rotationFlags;
		}
	};

	inline void to_json(nlohmann::json& j, const KSHLayerInfo& layer)
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

	struct KSHMovieInfo
	{
		std::string filename; // UTF-8 guaranteed
		std::int64_t offsetMs = 0;

		bool empty() const
		{
			return filename.empty() && offsetMs == 0;
		}
	};

	inline void to_json(nlohmann::json& j, const KSHMovieInfo& movie)
	{
		j = {
			{ "filename", movie.filename },
			{ "offset", movie.offsetMs },
		};
	}

	struct LegacyBGRoot
	{
		// first index: when gauge < 70%, second index: when gauge >= 70%
		std::array<KSHBGInfo, 2> bgInfos;
		std::array<KSHLayerInfo, 2> layerInfos;

		KSHMovieInfo movieInfos;

		bool empty() const
		{
			return bgInfos.empty() && layerInfos.empty() && movieInfos.empty();
		}
	};

	inline void to_json(nlohmann::json& j, const LegacyBGRoot& legacy)
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

	struct BGRoot
	{
		// KSH format will be legacy
		LegacyBGRoot legacy;
	};

	inline void to_json(nlohmann::json& j, const BGRoot& bg)
	{
		j = nlohmann::json::object();

		if (!bg.legacy.empty())
		{
			j["legacy"] = bg.legacy;
		}
	}
}
