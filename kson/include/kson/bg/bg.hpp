#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct KSHRotationFlags
	{
		bool tiltAffected = false;
		bool spinAffected = false;

		bool operator==(const KSHRotationFlags& rhs) const;
	};

	void to_json(nlohmann::json& j, const KSHRotationFlags& flags);

	void from_json(const nlohmann::json& j, KSHRotationFlags& flags);

	struct KSHBGInfo
	{
		std::string filename; // UTF-8 guaranteed
		KSHRotationFlags rotationFlags = { true, false };

		bool operator==(const KSHBGInfo& rhs) const;
	};

	void to_json(nlohmann::json& j, const KSHBGInfo& bg);

	struct KSHLayerInfo
	{
		std::string filename; // UTF-8 guaranteed
		std::int64_t durationMs = 0;
		KSHRotationFlags rotationFlags = { true, true };

		bool operator==(const KSHLayerInfo& rhs) const;
	};

	void to_json(nlohmann::json& j, const KSHLayerInfo& layer);

	struct KSHMovieInfo
	{
		std::string filename; // UTF-8 guaranteed
		std::int64_t offsetMs = 0;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const KSHMovieInfo& movie);

	struct LegacyBGRoot
	{
		// first index: when gauge < 70%, second index: when gauge >= 70%
		std::array<KSHBGInfo, 2> bgInfos;
		std::array<KSHLayerInfo, 2> layerInfos;

		KSHMovieInfo movieInfos;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const LegacyBGRoot& legacy);

	struct BGRoot
	{
		// KSH format will be legacy
		LegacyBGRoot legacy;
	};

	void to_json(nlohmann::json& j, const BGRoot& bg);
}
