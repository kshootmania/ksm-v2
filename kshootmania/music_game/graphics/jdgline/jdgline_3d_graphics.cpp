#include "jdgline_3d_graphics.hpp"
#include "music_game/graphics/graphics_defines.hpp"

namespace
{
	using namespace MusicGame::Graphics;

	constexpr StringView kJdglineTextureFilename = U"cline.png";
	constexpr Float3 kPlaneCenter = { 0.0f, 2.6f, -kHighwayPlaneSize.y / 2 - 0.4f };
	constexpr Float2 kPlaneSize = { 344.0f / 8, 26.0f / 8 };
}

MusicGame::Graphics::Jdgline3DGraphics::Jdgline3DGraphics()
	: m_jdglineTexture(TextureAsset(kJdglineTextureFilename))
	, m_mesh(MeshData::Grid(kPlaneCenter, kPlaneSize, 2, 2))
{
}

void MusicGame::Graphics::Jdgline3DGraphics::draw3D(double tiltRadians) const
{
	// Draw judgment line texture into 3D plane
	const ScopedRenderStates3D blendState(BlendState::NonPremultiplied);
	const Transformer3D transform(JudgmentPlaneTransformMatrix(tiltRadians, kPlaneCenter));
	m_mesh.draw(m_jdglineTexture);
}
