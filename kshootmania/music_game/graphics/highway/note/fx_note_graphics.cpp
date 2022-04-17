#include "fx_note_graphics.hpp"
#include "note_graphics_utils.hpp"
#include "music_game/graphics/graphics_defines.hpp"

namespace
{
	constexpr StringView kChipFXNoteTextureFilename = U"fx_chip.gif";
	constexpr StringView kLongFXNoteTextureFilename = U"fx_long.gif";
}

MusicGame::Graphics::FXNoteGraphics::FXNoteGraphics()
	: m_chipFXNoteTexture(NoteGraphicsUtils::ApplyAlphaToNoteTexture(TextureAsset(kChipFXNoteTextureFilename),
		{
			.column = 2,
			.sourceSize = { 82, 14 },
		}))
	, m_longFXNoteTexture(TextureAsset(kLongFXNoteTextureFilename))
{
}

void MusicGame::Graphics::FXNoteGraphics::draw(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const
{
	if (updateInfo.pChartData == nullptr)
	{
		return;
	}

	const ScopedRenderTarget2D renderTarget(additiveTarget);
	const ksh::ChartData& chartData = *updateInfo.pChartData;

	const double textureHeight = static_cast<double>(kHighwayTextureSize.y);

	for (std::size_t laneIdx = 0; laneIdx < ksh::kNumFXLanes; ++laneIdx)
	{
		const auto& lane = chartData.note.fxLanes[laneIdx];
		for (const auto& [y, note] : lane)
		{
			if (y + note.length < updateInfo.currentPulse - chartData.beat.resolution)
			{
				continue;
			}

			const double positionStartY = static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(y - updateInfo.currentPulse) * 480 / chartData.beat.resolution;
			if (positionStartY < 0)
			{
				break;
			}

			if (note.length == 0)
			{
				// Chip FX notes
				const double yRate = (static_cast<double>(kHighwayTextureSize.y) - positionStartY) / static_cast<double>(kHighwayTextureSize.y);
				m_chipFXNoteTexture()
					.resized(82, NoteGraphicsUtils::ChipNoteHeight(yRate))
					.draw(kLanePositionOffset + kFXLanePositionDiff * laneIdx + Vec2::Down(positionStartY));
			}
			else
			{
				// Long FX notes
				const double positionEndY = static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(y + note.length - updateInfo.currentPulse) * 480 / chartData.beat.resolution;
				m_longFXNoteTexture(0, 0, 82, 1)
					.resized(82, note.length * 480 / chartData.beat.resolution)
					.draw(kLanePositionOffset + kFXLanePositionDiff * laneIdx + Vec2::Down(positionEndY));
			}
		}
	}
}
