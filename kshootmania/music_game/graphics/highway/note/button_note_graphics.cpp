#include "button_note_graphics.hpp"
#include "note_graphics_utils.hpp"
#include "music_game/graphics/graphics_defines.hpp"

namespace
{
	constexpr StringView kChipBTNoteTextureFilename = U"bt_chip.gif";
	constexpr StringView kLongBTNoteTextureFilename = U"bt_long.gif";

	constexpr StringView kChipFXNoteTextureFilename = U"fx_chip.gif";
	constexpr StringView kLongFXNoteTextureFilename = U"fx_long.gif";

	constexpr double kLongNoteSourceYDefault = 0.0;
	constexpr double kLongNoteSourceYPressed1 = 8.0;
	constexpr double kLongNoteSourceYPressed2 = 9.0;
	constexpr double kLongNoteSourceYNotPressed = 10.0;

	double PressedLongNoteSourceY(double currentTimeSec)
	{
		return (MathUtils::WrappedFmod(currentTimeSec, 0.1) < 0.05) ? kLongNoteSourceYPressed1 : kLongNoteSourceYPressed2;
	}
}

void MusicGame::Graphics::ButtonNoteGraphics::drawLongNotesCommon(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget, bool isBT) const
{
	const ksh::ChartData& chartData = *updateInfo.pChartData;
	const double highwayTextureHeight = static_cast<double>(kHighwayTextureSize.y);

	for (std::size_t laneIdx = 0; laneIdx < (isBT ? ksh::kNumBTLanes : ksh::kNumFXLanes); ++laneIdx)
	{
		const auto& lane = isBT ? chartData.note.btLanes[laneIdx] : chartData.note.fxLanes[laneIdx];
		for (const auto& [y, note] : lane)
		{
			if (y + note.length < updateInfo.currentPulse - chartData.beat.resolution)
			{
				continue;
			}

			const double positionStartY = highwayTextureHeight - static_cast<double>(y - updateInfo.currentPulse) * 480 / chartData.beat.resolution;
			if (positionStartY < 0)
			{
				break;
			}

			if (note.length > 0)
			{
				const double positionEndY = highwayTextureHeight - static_cast<double>(y + note.length - updateInfo.currentPulse) * 480 / chartData.beat.resolution;
				for (int32 i = 0; i < (isBT ? 2 : 1); ++i) // Note: Long BT note has additional texture for invMultiplyTarget
				{
					const ScopedRenderTarget2D renderTarget((i == 0) ? additiveTarget : invMultiplyTarget);
					const ScopedRenderStates2D blendState((i == 0) ? (isBT ? BlendState::Additive : BlendState::Default2D) : BlendState::Subtractive);
					const LaneState& laneState = isBT ? updateInfo.btLaneState[laneIdx] : updateInfo.fxLaneState[laneIdx];
					double sourceY;
					if (laneState.currentLongNotePulse == y)
					{
						// Current note and pressed
						sourceY = PressedLongNoteSourceY(updateInfo.currentTimeSec);
					}
					else if (y <= updateInfo.currentPulse && updateInfo.currentPulse < y + note.length)
					{
						// Current note but not pressed
						sourceY = kLongNoteSourceYNotPressed;
					}
					else
					{
						// Not current note
						sourceY = kLongNoteSourceYDefault;
					}
					const Texture& sourceTexture = isBT ? m_longBTNoteTexture : m_longFXNoteTexture;
					const int32 width = isBT ? 40 : 82;
					const Vec2 position = kLanePositionOffset + (isBT ? kBTLanePositionDiff : kFXLanePositionDiff) * laneIdx + Vec2::Down(positionEndY);
					sourceTexture(width * i, sourceY + kOnePixelTextureSourceOffset, width, kOnePixelTextureSourceSize)
						.resized(width, static_cast<double>(note.length) * 480 / chartData.beat.resolution)
						.draw(position);
				}
			}
		}
	}
}

void MusicGame::Graphics::ButtonNoteGraphics::drawLongBTNotes(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const
{
	drawLongNotesCommon(updateInfo, additiveTarget, invMultiplyTarget, true);
}

void MusicGame::Graphics::ButtonNoteGraphics::drawLongFXNotes(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const
{
	drawLongNotesCommon(updateInfo, additiveTarget, invMultiplyTarget, false);
}

MusicGame::Graphics::ButtonNoteGraphics::ButtonNoteGraphics()
	: m_chipBTNoteTexture(NoteGraphicsUtils::ApplyAlphaToNoteTexture(TextureAsset(kChipBTNoteTextureFilename),
		{
			.column = 9 * 2,
			.sourceSize = { 40, 14 },
		}))
	, m_longBTNoteTexture(TextureAsset(kLongBTNoteTextureFilename))
	, m_chipFXNoteTexture(NoteGraphicsUtils::ApplyAlphaToNoteTexture(TextureAsset(kChipFXNoteTextureFilename),
		{
			.column = 2,
			.sourceSize = { 82, 14 },
		}))
	, m_longFXNoteTexture(TextureAsset(kLongFXNoteTextureFilename))
{
}

void MusicGame::Graphics::ButtonNoteGraphics::draw(const UpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const
{
	if (updateInfo.pChartData == nullptr)
	{
		return;
	}

	const ScopedRenderStates2D defaultSamplerState(SamplerState::ClampNearest);
	const ScopedRenderTarget2D defaultRenderTarget(additiveTarget);
	const ksh::ChartData& chartData = *updateInfo.pChartData;

	const double highwayTextureHeight = static_cast<double>(kHighwayTextureSize.y);

	drawLongBTNotes(updateInfo, additiveTarget, invMultiplyTarget);

	drawLongFXNotes(updateInfo, additiveTarget, invMultiplyTarget);

	// Chip FX notes
	for (std::size_t laneIdx = 0; laneIdx < ksh::kNumFXLanes; ++laneIdx)
	{
		const auto& lane = chartData.note.fxLanes[laneIdx];
		for (const auto& [y, note] : lane)
		{
			if (y + note.length < updateInfo.currentPulse - chartData.beat.resolution)
			{
				continue;
			}

			const double positionStartY = highwayTextureHeight - static_cast<double>(y - updateInfo.currentPulse) * 480 / chartData.beat.resolution;
			if (positionStartY < 0)
			{
				break;
			}

			const double dLaneIdx = static_cast<double>(laneIdx);

			if (note.length == 0)
			{
				const double yRate = (highwayTextureHeight - positionStartY) / highwayTextureHeight;
				const double height = NoteGraphicsUtils::ChipNoteHeight(yRate);
				m_chipFXNoteTexture()
					.resized(82, NoteGraphicsUtils::ChipNoteHeight(yRate))
					.draw(kLanePositionOffset + kFXLanePositionDiff * dLaneIdx + Vec2::Down(positionStartY - height / 2));
			}
		}
	}

	// Chip BT notes
	for (std::size_t laneIdx = 0; laneIdx < ksh::kNumBTLanes; ++laneIdx)
	{
		const auto& lane = chartData.note.btLanes[laneIdx];
		for (const auto& [y, note] : lane)
		{
			if (y + note.length < updateInfo.currentPulse - chartData.beat.resolution)
			{
				continue;
			}

			const double positionStartY = highwayTextureHeight - static_cast<double>(y - updateInfo.currentPulse) * 480 / chartData.beat.resolution;
			if (positionStartY < 0)
			{
				break;
			}

			const double dLaneIdx = static_cast<double>(laneIdx);

			if (note.length == 0)
			{
				const double yRate = (highwayTextureHeight - positionStartY) / highwayTextureHeight;
				const double height = NoteGraphicsUtils::ChipNoteHeight(yRate);
				m_chipBTNoteTexture()
					.resized(40, height)
					.draw(kLanePositionOffset + kBTLanePositionDiff * dLaneIdx + Vec2::Down(positionStartY - height / 2));
			}
		}
	}
}
