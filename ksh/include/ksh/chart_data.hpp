#pragma once
#include "ksh/common/common.hpp"
#include "ksh/meta/meta.hpp"
#include "ksh/beat/beat_map.hpp"
#include "ksh/gauge/gauge.hpp"
#include "ksh/note/note.hpp"
#include "ksh/camera/camera.hpp"
#include "ksh/bg/bg.hpp"

namespace ksh
{
	struct ChartData
	{
		MetaRoot meta;
		BeatMap beat;
		GaugeRoot gauge;
		NoteRoot note;
		CameraRoot camera;
		BGRoot bg;
	};
}
