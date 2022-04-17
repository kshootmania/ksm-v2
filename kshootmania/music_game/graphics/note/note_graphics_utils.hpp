#pragma once
#include "music_game/graphics/update_info/update_info.hpp"

namespace MusicGame::Graphics
{
	namespace NoteGraphicsUtils
	{
		TiledTexture ApplyAlphaToNoteTexture(const Texture& texture, const TiledTextureSizeInfo& sizeInfo);

		double ChipNoteHeight(double yRate);
	}
}
