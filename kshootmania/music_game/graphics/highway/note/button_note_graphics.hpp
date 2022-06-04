#pragma once
#include "music_game/graphics/graphics_update_info.hpp"

namespace MusicGame::Graphics
{
	class ButtonNoteGraphics
	{
	private:
		const TiledTexture m_chipBTNoteTexture;
		const Texture m_longBTNoteTexture;

		const TiledTexture m_chipFXNoteTexture;
		const Texture m_longFXNoteTexture;

		void drawChipNotesCommon(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget, bool isBT) const;

		void drawChipBTNotes(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget) const;

		void drawChipFXNotes(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget) const;

		void drawLongNotesCommon(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget, bool isBT) const;

		void drawLongBTNotes(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const;

		void drawLongFXNotes(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const;

	public:
		ButtonNoteGraphics();

		void draw(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const;
	};
}
