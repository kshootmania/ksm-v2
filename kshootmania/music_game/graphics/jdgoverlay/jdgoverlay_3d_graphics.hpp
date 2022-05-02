#pragma once
#include "music_game/graphics/update_info.hpp"

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

		void drawChipAnimCommon(const UpdateInfo& updateInfo, bool isBT) const;

		void drawChipAnimBT(const UpdateInfo& updateInfo) const;

		void drawChipAnimFX(const UpdateInfo& updateInfo) const;

		void drawLongAnimCommon(const UpdateInfo& updateInfo, bool isBT) const;

		void drawLongAnimBT(const UpdateInfo& updateInfo) const;

		void drawLongAnimFX(const UpdateInfo& updateInfo) const;

	public:
		Jdgoverlay3DGraphics();

		void draw2D(const UpdateInfo& updateInfo) const;

		void draw3D(double tiltRadians) const;
	};
}
