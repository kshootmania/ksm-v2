#pragma once
#include "music_game/game_status.hpp"

namespace MusicGame::Graphics
{
	namespace NoteGraphicsUtils
	{
		TiledTexture ApplyAlphaToNoteTexture(const Texture& texture, const TiledTextureSizeInfo& sizeInfo);

		double ChipNoteHeight(double yRate);
	}
}
