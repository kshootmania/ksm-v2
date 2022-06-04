#pragma once
#include "music_game/graphics/graphics_update_info.hpp"
#include "note/button_note_graphics.hpp"
#include "note/laser_note_graphics.hpp"
#include "key_beam_graphics.hpp"

namespace MusicGame::Graphics
{
	class Highway3DGraphics
	{
	private:
		const Texture m_baseTexture;
		const Texture m_shineEffectTexture;

		RenderTexture m_additiveRenderTexture;
		RenderTexture m_invMultiplyRenderTexture;

		ButtonNoteGraphics m_buttonNoteGraphics;
		LaserNoteGraphics m_laserNoteGraphics;

		KeyBeamGraphics m_keyBeamGraphics;

		MeshData m_meshData;
		DynamicMesh m_mesh;

	public:
		Highway3DGraphics();

		void draw2D(const GraphicsUpdateInfo& updateInfo) const;

		void draw3D(double tiltRadians) const;
	};
}
