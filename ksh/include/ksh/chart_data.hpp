#pragma once
#include "ksh/common/common.hpp"
#include "ksh/meta/meta.hpp"
#include "ksh/beat/beat_map.hpp"
#include "ksh/gauge/gauge.hpp"
#include "ksh/note/note.hpp"
#include "ksh/audio/audio.hpp"
#include "ksh/camera/camera.hpp"
#include "ksh/bg/bg.hpp"
#include "ksh/impl/impl.hpp"

namespace ksh
{
	enum class Error
	{
		kNone = 0,
		kFileNotFound,
		kCannotOpenFileStream,
		kChartFormatError,
		kUnknownError,
	};

	struct MetaChartData
	{
		MetaRoot meta;
		MetaAudioRoot audio;

		Error error = Error::kNone;
	};

	struct ChartData
	{
		MetaRoot meta;
		BeatMap beat;
		GaugeRoot gauge;
		NoteRoot note;
		AudioRoot audio;
		CameraRoot camera;
		BGRoot bg;
		ImplRoot impl;

		Error error = Error::kNone;
	};

	inline void to_json(nlohmann::json& j, const ChartData& chartData)
	{
		j = {
			{ "meta", chartData.meta },
			{ "beat", chartData.beat },
			{ "note", chartData.note },
			{ "audio", chartData.audio },
			{ "bg", chartData.bg },
		};

		if (!chartData.gauge.empty())
		{
			j["gauge"] = chartData.gauge;
		}

		if (!chartData.camera.empty())
		{
			j["camera"] = chartData.camera;
		}
	}
}
