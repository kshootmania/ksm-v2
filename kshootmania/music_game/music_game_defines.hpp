#pragma once

namespace MusicGame
{
	constexpr double kTimeSecBeforeStart = 3.4;
	constexpr double kTimeSecBeforeStartMovie = 4.4;

	constexpr double TimeSecBeforeStart(bool movie)
	{
		return movie ? kTimeSecBeforeStartMovie : kTimeSecBeforeStart;
	}
}
