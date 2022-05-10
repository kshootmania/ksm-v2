#pragma once
#include "kson/common/common.hpp"
#include "kson/meta/meta.hpp"
#include "kson/beat/beat_map.hpp"
#include "kson/gauge/gauge.hpp"
#include "kson/note/note.hpp"
#include "kson/audio/audio.hpp"
#include "kson/camera/camera.hpp"
#include "kson/bg/bg.hpp"
#include "kson/editor/editor_info.hpp"
#include "kson/compat/compat_info.hpp"

namespace kson
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
		MetaCompatInfo compat;

		std::string filePath; // Note: OS native encoding (Not UTF-8 in Windows)
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
		EditorInfo editor;
		CompatInfo compat;
		nlohmann::json impl;

		std::string filePath; // Note: OS native encoding (Not UTF-8 in Windows)
		Error error = Error::kNone;
	};

	void to_json(nlohmann::json& j, const ChartData& chartData);
}
