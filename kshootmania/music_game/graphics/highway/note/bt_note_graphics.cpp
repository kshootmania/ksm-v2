#include "bt_note_graphics.hpp"
#include "note_graphics_utils.hpp"
#include "music_game/graphics/graphics_defines.hpp"

namespace
{
	constexpr StringView kChipBTNoteTextureFilename = U"bt_chip.gif";
	constexpr StringView kLongBTNoteTextureFilename = U"bt_long.gif";
}

MusicGame::Graphics::BTNoteGraphics::BTNoteGraphics()
	: m_chipBTNoteTexture(NoteGraphicsUtils::ApplyAlphaToNoteTexture(TextureAsset(kChipBTNoteTextureFilename),
		{
			.column = 9 * 2,
			.sourceSize = { 40, 14 },
		}))
	, m_longBTNoteTexture(TextureAsset(kLongBTNoteTextureFilename))
{
}

void MusicGame::Graphics::BTNoteGraphics::draw(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const
{
	if (updateInfo.pChartData == nullptr)
	{
		return;
	}

	const ScopedRenderTarget2D defaultRenderTarget(additiveTarget);
	const ksh::ChartData& chartData = *updateInfo.pChartData;

	const double textureHeight = static_cast<double>(kHighwayTextureSize.y);

	for (std::size_t laneIdx = 0; laneIdx < ksh::kNumBTLanes; ++laneIdx)
	{
		const auto& lane = chartData.note.btLanes[laneIdx];
		for (const auto& [y, note] : lane)
		{
			if (y + note.length < updateInfo.currentPulse - chartData.beat.resolution)
			{
				continue;
			}

			const double positionStartY = textureHeight - static_cast<double>(y - updateInfo.currentPulse) * 480 / chartData.beat.resolution;
			if (positionStartY < 0)
			{
				break;
			}

			const double dLaneIdx = static_cast<double>(laneIdx);

			if (note.length == 0)
			{
				// Chip BT notes
				const double yRate = (textureHeight - positionStartY) / textureHeight;
				m_chipBTNoteTexture()
					.resized(40, NoteGraphicsUtils::ChipNoteHeight(yRate))
					.draw(kLanePositionOffset + kBTLanePositionDiff * dLaneIdx + Vec2::Down(positionStartY));
			}
			else
			{
				// Long BT notes
				const ScopedRenderStates2D renderState(BlendState::Additive);
				const double positionEndY = textureHeight - static_cast<double>(y + note.length - updateInfo.currentPulse) * 480 / chartData.beat.resolution;
				for (int32 i = 0; i < 2; ++i)
				{
					const ScopedRenderTarget2D renderTarget((i == 0) ? additiveTarget : invMultiplyTarget);
					m_longBTNoteTexture(40 * i, 0, 40, 1)
						.resized(40, static_cast<double>(note.length) * 480 / chartData.beat.resolution)
						.draw(kLanePositionOffset + kBTLanePositionDiff * dLaneIdx + Vec2::Down(positionEndY));
				}
			}
		}
	}
}
