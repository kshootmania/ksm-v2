#pragma once
#include "camera_state.hpp"

namespace MusicGame
{
	class Highway3DGraphics
	{
	private:
		MSRenderTexture m_3dViewTexture;

		const Texture m_bgTexture;

		MeshData m_additiveMeshData;
		DynamicMesh m_additiveMesh;

		MeshData m_subtractiveMeshData;
		DynamicMesh m_subtractiveMesh;

		DebugCamera3D m_debugCamera;

	public:
		Highway3DGraphics();

		void update(const CameraState& cameraState);

		void draw() const;
	};
}
