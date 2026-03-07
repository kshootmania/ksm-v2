#pragma once
#include "MusicGame/GameStatus.hpp"
#include "MusicGame/ViewStatus.hpp"
#include "MusicGame/PlayOption.hpp"
#include "MusicGame/Scroll/HighwayScroll.hpp"
#include "HighwayRenderTexture.hpp"
#include "KeyBeamGraphics.hpp"
#include "Note/ButtonNoteGraphics.hpp"
#include "Note/LaserNoteGraphics.hpp"

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

		void draw2D(const kson::ChartData& chartData, const std::array<HashSet<kson::Pulse>, kson::kNumLaserLanesSZ>& laserCurvedPulses, const PlayOption& playOption, const kson::TimingCache& timingCache, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext) const;

		void draw3D(const ViewStatus& viewStatus) const;
	};
}
