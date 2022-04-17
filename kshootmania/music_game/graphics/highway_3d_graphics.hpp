#pragma once
#include "update_info/update_info.hpp"
#include "highway_tilt.hpp"

namespace MusicGame::Graphics
{
	class Highway3DGraphics
	{
	private:
		const Texture m_bgTexture;
		const Texture m_shineEffectTexture;
		const Texture m_beamTexture;
		const Texture m_laserNoteTexture;
		const Texture m_laserNoteMaskTexture;
		const TiledTexture m_laserNoteLeftStartTexture;
		const TiledTexture m_laserNoteRightStartTexture;

		RenderTexture m_additiveRenderTexture;
		RenderTexture m_invMultiplyRenderTexture;

		const std::array<std::pair<std::reference_wrapper<const RenderTexture>, std::reference_wrapper<const Texture>>, 2> m_drawTexturePairs;

		MeshData m_meshData;
		DynamicMesh m_mesh;

		HighwayTilt m_highwayTilt;

	public:
		Highway3DGraphics();

		void update(const UpdateInfo& updateInfo);

		void draw(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& subtractiveTarget) const;
	};
}
