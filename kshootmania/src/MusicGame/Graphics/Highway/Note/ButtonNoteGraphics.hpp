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

		// キー音付きFXチップノーツのpulse(配置後のレーン基準)
		const std::array<HashSet<kson::Pulse>, kNumSRandomLanesSZ> m_fxChipKeySoundPulses;

		// レーン毎の最大ノーツ長
		const std::array<kson::RelPulse, kson::kNumBTLanesSZ> m_btMaxNoteLengths;
		const std::array<kson::RelPulse, kson::kNumFXLanesSZ> m_fxMaxNoteLengths;

		void drawChipNotesCommon(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target, bool isBT) const;

		void drawChipBTNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;

		void drawChipFXNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;

		void drawLongNotesCommon(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target, bool isBT) const;

		void drawLongBTNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;

		void drawLongFXNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;

	public:
		explicit ButtonNoteGraphics(const kson::ChartData& chartData, const std::array<HashSet<kson::Pulse>, kNumSRandomLanesSZ>& fxChipKeySoundPulses);

		void draw(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;
	};
}
