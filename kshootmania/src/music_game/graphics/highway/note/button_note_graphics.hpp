#pragma once
#include "music_game/game_status.hpp"
#include "music_game/view_status.hpp"
#include "music_game/scroll/highway_scroll.hpp"
#include "music_game/graphics/highway/highway_render_texture.hpp"

namespace MusicGame::Graphics
{
	class ButtonNoteGraphics
	{
	private:
		const TiledTexture m_chipBTNoteTexture;
		const Texture m_longBTNoteTexture;

		const TiledTexture m_chipFXNoteTexture;
		const Texture m_longFXNoteTexture;

		void drawChipNotesCommon(const kson::ChartData& chartData, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target, bool isBT) const;

		void drawChipBTNotes(const kson::ChartData& chartData, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;

		void drawChipFXNotes(const kson::ChartData& chartData, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;

		void drawLongNotesCommon(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target, bool isBT) const;

		void drawLongBTNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;

		void drawLongFXNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;

	public:
		ButtonNoteGraphics();

		void draw(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;
	};
}
