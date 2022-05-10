#include "kson/compat/compat_info.hpp"

bool kson::KSHUnknownInfo::empty() const
{
	return meta.empty() && option.empty() && line.empty();
}

bool kson::CompatInfo::empty() const
{
	return kshVersion.empty() && kshUnknown.empty();
}

void kson::to_json(nlohmann::json& j, const KSHUnknownInfo& unknown)
{
	j = nlohmann::json::object();

	if (!unknown.meta.empty())
	{
		j["meta"] = unknown.meta;
	}

	if (!unknown.option.empty())
	{
		auto json = nlohmann::json::object();
		for (const auto& [key, options] : unknown.option)
		{
			auto subJson = nlohmann::json::array();
			for (const auto& [y, value] : options)
			{
				subJson.push_back({
					{ "y", y },
					{ "v", value },
				});
			}
			json[key] = subJson;
		}
		j["option"] = json;
	}

	if (!unknown.line.empty())
	{
		auto json = nlohmann::json::array();
		for (const auto& [y, line] : unknown.line)
		{
			json.push_back({
				{ "y", y },
				{ "v", line },
			});
		}
		j["line"] = json;
	}
}

void kson::to_json(nlohmann::json& j, const CompatInfo& compat)
{
	j = nlohmann::json::object();

	if (!compat.kshVersion.empty())
	{
		j["ksh_version"] = compat.kshVersion;
	}

	if (!compat.kshUnknown.empty())
	{
		j["ksh_unknown"] = compat.kshUnknown;
	}
}
