#pragma once
#include "music_game/camera/cam_system.hpp"

namespace MusicGame::Camera
{
	struct CamPatternStatusSpin
	{
	private:
		const kson::Pulse m_startPulse;

		const kson::CamPatternInvokeSpin m_invoke;

	public:
		CamPatternStatusSpin(kson::Pulse startPulse, const kson::CamPatternInvokeSpin& invoke);

		void apply(kson::Pulse currentPulse, CamStatus& camStatusRef) const;
	};
}
