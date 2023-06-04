#include "jdgline_3d_graphics.hpp"
#include "music_game/graphics/graphics_defines.hpp"
#include "music_game/camera/camera_math.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kJdglineTextureFilename = U"cline.png";
		constexpr Float3 kPlaneCenter = { 0.0f, 2.6f, -kHighwayPlaneSize.y / 2 - 0.4f };
		constexpr Float2 kPlaneSize = { 344.0f / 8, 26.0f / 8 };
	}

	Jdgline3DGraphics::Jdgline3DGraphics()
		: m_jdglineTexture(TextureAsset(kJdglineTextureFilename))
		, m_mesh(MeshData::Grid(Float3::Zero(), kPlaneSize, 2, 2))
	{
	}

	void Jdgline3DGraphics::draw3D(const GameStatus& gameStatus, const ViewStatus& viewStatus) const
	{
		// 3Dの板に判定ラインのテクスチャを描画
		const ScopedRenderStates3D blendState(BlendState::NonPremultiplied);
		const double jdgoverlayScale = Camera::JdgoverlayScale(viewStatus.camStatus.zoom);
		const double jdglineScale = Camera::JdglineScale(viewStatus.camStatus.zoom);
		const double shiftX = viewStatus.camStatus.shiftX;
		const Vec3 shiftXVec = Vec3::Right(Camera::ScaledCamShiftXValue(shiftX) * jdgoverlayScale);
		const double radians = Math::ToRadians(viewStatus.camStatus.rotationZJdgline) + viewStatus.tiltRadians;
		const Transformer3D transform(Mat4x4::Scale(jdglineScale) * Mat4x4::Translate(shiftXVec) * JudgmentPlaneTransformMatrix(radians) * Mat4x4::Translate(kPlaneCenter));
		m_mesh.draw(m_jdglineTexture);
	}
}
