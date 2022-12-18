#pragma once
#include "music_game/game_status.hpp"

namespace MusicGame::Graphics
{
	class ButtonNoteGraphics
	{
	private:
		const TiledTexture m_chipBTNoteTexture;
		const Texture m_longBTNoteTexture;

		const TiledTexture m_chipFXNoteTexture;
		const Texture m_longFXNoteTexture;

		void drawChipNotesCommon(const kson::ChartData& chartData, const GameStatus& gameStatus, const RenderTexture& additiveTarget, bool isBT) const;

		void drawChipBTNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const RenderTexture& additiveTarget) const;

		void drawChipFXNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const RenderTexture& additiveTarget) const;

		void drawLongNotesCommon(const kson::ChartData& chartData, const GameStatus& gameStatus, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget, bool isBT) const;

		void drawLongBTNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const;

		void drawLongFXNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const;

	public:
		ButtonNoteGraphics();

		void draw(const kson::ChartData& chartData, const GameStatus& gameStatus, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const;
	};
}
