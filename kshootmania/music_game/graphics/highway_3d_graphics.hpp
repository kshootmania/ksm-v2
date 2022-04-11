#pragma once
#include "update_info/update_info.hpp"

namespace MusicGame::Graphics
{
	class Highway3DGraphics
	{
	private:
		const Texture m_bgTexture;
		const Texture m_shineEffectTexture;
		const Texture m_beamTexture;
		const TiledTexture m_chipBTNoteTexture;
		const Texture m_longBTNoteTexture;
		const TiledTexture m_chipFXNoteTexture;
		const Texture m_longFXNoteTexture;

		RenderTexture m_additiveRenderTexture;
		RenderTexture m_invMultiplyRenderTexture;

		MeshData m_meshData;
		DynamicMesh m_mesh;

		UpdateInfo m_updateInfo;

	public:
		Highway3DGraphics();

		void update(const UpdateInfo& updateInfo);

		void draw(const RenderTexture& additiveTarget, const RenderTexture& subtractiveTarget) const;
	};
}
