#pragma once
#include "kson/common/common.hpp"
#include "kson/meta/meta_info.hpp"
#include "kson/beat/beat_map.hpp"
#include "kson/gauge/gauge_info.hpp"
#include "kson/note/note_info.hpp"
#include "kson/audio/audio_info.hpp"
#include "kson/camera/camera_info.hpp"
#include "kson/bg/bg_info.hpp"
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
		MetaInfo meta;
		MetaAudioInfo audio;
		MetaCompatInfo compat;

		std::string filePath; // Note: OS native encoding (Not UTF-8 in Windows)
		Error error = Error::kNone;
	};

	struct ChartData
	{
		MetaInfo meta;
		BeatMap beat;
		GaugeInfo gauge;
		NoteInfo note;
		AudioInfo audio;
		CameraInfo camera;
		BGInfo bg;
		EditorInfo editor;
		CompatInfo compat;
		nlohmann::json impl;

		std::string filePath; // Note: OS native encoding (Not UTF-8 in Windows)
		Error error = Error::kNone;
	};

	void to_json(nlohmann::json& j, const ChartData& chartData);
}
