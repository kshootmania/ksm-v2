#pragma once
#include "music_game/graphics/graphics_update_info.hpp"

namespace MusicGame::Graphics
{
	class Jdgoverlay3DGraphics
	{
	private:
		const RenderTexture m_renderTexture;

		const TiledTexture m_chipCriticalTexture;
		const TiledTexture m_chipNearTexture;
		const TiledTexture m_chipErrorTexture;

		const TiledTexture m_longAnimTexture;

		const Mesh m_mesh;

		const TiledTexture& chipAnimTexture(Judgment::JudgmentResult type) const;

		void drawChipAnimCommon(const GraphicsUpdateInfo& updateInfo, bool isBT) const;

		void drawChipAnimBT(const GraphicsUpdateInfo& updateInfo) const;

		void drawChipAnimFX(const GraphicsUpdateInfo& updateInfo) const;

		void drawLongAnimCommon(const GraphicsUpdateInfo& updateInfo, bool isBT) const;

		void drawLongAnimBT(const GraphicsUpdateInfo& updateInfo) const;

		void drawLongAnimFX(const GraphicsUpdateInfo& updateInfo) const;

	public:
		Jdgoverlay3DGraphics();

		void draw2D(const GraphicsUpdateInfo& updateInfo) const;

		void draw3D(double tiltRadians) const;
	};
}
