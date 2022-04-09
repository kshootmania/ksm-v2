#pragma once
#include "camera_state.hpp"

namespace MusicGame
{
	class Highway3DGraphics
	{
	private:
		const Texture m_bgTexture;
		RenderTexture m_additiveRenderTexture;
		RenderTexture m_subtractiveRenderTexture;

		MeshData m_meshData;
		DynamicMesh m_mesh;

	public:
		Highway3DGraphics();

		void update(const CameraState& cameraState);

		void draw(const RenderTexture& target) const;
	};
}
