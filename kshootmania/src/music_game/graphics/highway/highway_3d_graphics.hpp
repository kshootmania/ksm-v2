#pragma once
#include "music_game/game_status.hpp"
#include "music_game/view_status.hpp"
#include "music_game/scroll/highway_scroll.hpp"
#include "highway_render_texture.hpp"
#include "key_beam_graphics.hpp"
#include "note/button_note_graphics.hpp"
#include "note/laser_note_graphics.hpp"

namespace MusicGame::Graphics
{
	class Highway3DGraphics
	{
	private:
		const Texture m_shineEffectTexture;
		const Texture m_barLineTexture;

		const HighwayRenderTexture m_renderTexture;

		const ButtonNoteGraphics m_buttonNoteGraphics;
		const LaserNoteGraphics m_laserNoteGraphics;

		const KeyBeamGraphics m_keyBeamGraphics;

		MeshData m_meshData;
		DynamicMesh m_mesh;
		bool m_trianglesFlipped = false;

	public:
		Highway3DGraphics();

		void update(const ViewStatus& viewStatus);

		void draw2D(const kson::ChartData& chartData, const kson::TimingCache& timingCache, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext) const;

		void draw3D(const GameStatus& gameStatus, const ViewStatus& viewStatus) const;
	};
}
