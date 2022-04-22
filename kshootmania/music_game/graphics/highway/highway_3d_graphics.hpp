#pragma once
#include "music_game/graphics/update_info.hpp"
#include "note/bt_note_graphics.hpp"
#include "note/fx_note_graphics.hpp"
#include "note/laser_note_graphics.hpp"
#include "key_beam_graphics.hpp"

namespace MusicGame::Graphics
{
	class Highway3DGraphics
	{
	private:
		const Texture m_bgTexture;
		const Texture m_shineEffectTexture;

		RenderTexture m_additiveRenderTexture;
		RenderTexture m_invMultiplyRenderTexture;

		BTNoteGraphics m_btNoteGraphics;
		FXNoteGraphics m_fxNoteGraphics;
		LaserNoteGraphics m_laserNoteGraphics;

		KeyBeamGraphics m_keyBeamGraphics;

		MeshData m_meshData;
		DynamicMesh m_mesh;

	public:
		Highway3DGraphics();

		void draw(const UpdateInfo& updateInfo, double tiltRadians) const;
	};
}
