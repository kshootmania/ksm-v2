#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct KSHLayerRotationInfo
	{
		bool tilt = false;
		bool spin = false;
	};

	void to_json(nlohmann::json& j, const KSHLayerRotationInfo& flags);

	void from_json(const nlohmann::json& j, KSHLayerRotationInfo& flags);

	struct KSHBGInfo
	{
		std::string filename; // UTF-8 guaranteed

		bool operator==(const KSHBGInfo& rhs) const;
	};

	void to_json(nlohmann::json& j, const KSHBGInfo& bg);

	struct KSHLayerInfo
	{
		std::string filename; // UTF-8 guaranteed
		std::int32_t duration = 0;
		KSHLayerRotationInfo rotation = { true, true };

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const KSHLayerInfo& layer);

	struct KSHMovieInfo
	{
		std::string filename; // UTF-8 guaranteed
		std::int32_t offset = 0;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const KSHMovieInfo& movie);

	struct LegacyBGInfo
	{
		// first index: when gauge < 70%, second index: when gauge >= 70%
		std::array<KSHBGInfo, 2> bg;

		KSHLayerInfo layer;

		KSHMovieInfo movie;

		bool empty() const;
	};

	void to_json(nlohmann::json& j, const LegacyBGInfo& legacy);

	struct BGInfo
	{
		// KSH format will be legacy
		LegacyBGInfo legacy;
	};

	void to_json(nlohmann::json& j, const BGInfo& bg);
}
