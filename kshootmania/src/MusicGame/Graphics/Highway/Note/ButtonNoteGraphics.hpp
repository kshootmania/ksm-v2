#pragma once
#include "MusicGame/GameStatus.hpp"
#include "MusicGame/ViewStatus.hpp"
#include "MusicGame/PlayOption.hpp"
#include "MusicGame/Scroll/HighwayScroll.hpp"
#include "MusicGame/Graphics/Highway/HighwayRenderTexture.hpp"

namespace MusicGame::Graphics
{
	class ButtonNoteGraphics
	{
	private:
		const TiledTexture m_chipBTNoteTexture;
		const Texture m_longBTNoteTexture;
		const Texture m_longBTNoteStartTexture;

		const TiledTexture m_chipFXNoteTexture;
		const TiledTexture m_chipFXSENoteTexture;
		const Texture m_longFXNoteTexture;
		const Texture m_longFXNoteStartTexture;

		void drawChipNotesCommon(const kson::ChartData& chartData, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target, bool isBT) const;

		void drawChipBTNotes(const kson::ChartData& chartData, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;

		void drawChipFXNotes(const kson::ChartData& chartData, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;

		void drawLongNotesCommon(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target, bool isBT) const;

		void drawLongBTNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;

		void drawLongFXNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;

	public:
		ButtonNoteGraphics();

		void draw(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;
	};
}
