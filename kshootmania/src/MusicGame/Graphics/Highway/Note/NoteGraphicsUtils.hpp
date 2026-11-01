#pragma once
#include "MusicGame/GameStatus.hpp"

namespace MusicGame::Graphics
{
	namespace NoteGraphicsUtils
	{
		TiledTexture ApplyAlphaToNoteTexture(const Texture& texture, const TiledTextureSizeInfo& sizeInfo);

		int32 ChipNoteHeight(double yRate);

		int32 CalcChipNoteColorIndex(kson::Pulse y, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache, NoteSkinType noteSkin);
	}
}
