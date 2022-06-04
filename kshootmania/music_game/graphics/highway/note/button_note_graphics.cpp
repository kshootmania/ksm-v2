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

void MusicGame::Graphics::ButtonNoteGraphics::drawChipNotesCommon(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget, bool isBT) const
{
	const kson::ChartData& chartData = *updateInfo.pChartData;
	const double highwayTextureHeight = static_cast<double>(kHighwayTextureSize.y);

	const ScopedRenderTarget2D renderTarget(additiveTarget);
	const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);

	for (std::size_t laneIdx = 0; laneIdx < (isBT ? kson::kNumBTLanes : kson::kNumFXLanes); ++laneIdx)
	{
		const auto& lane = isBT ? chartData.note.btLanes[laneIdx] : chartData.note.fxLanes[laneIdx];
		for (const auto& [y, note] : lane)
		{
			if (y + note.length < updateInfo.currentPulse - kson::kResolution)
			{
				continue;
			}

			const double positionStartY = highwayTextureHeight - static_cast<double>(y - updateInfo.currentPulse) * 480 / kson::kResolution;
			if (positionStartY < 0)
			{
				break;
			}

			if (note.length == 0)
			{
				const double yRate = (highwayTextureHeight - positionStartY) / highwayTextureHeight;
				const double height = NoteGraphicsUtils::ChipNoteHeight(yRate);
				const TiledTexture& sourceTexture = isBT ? m_chipBTNoteTexture : m_chipFXNoteTexture;
				const Vec2 position = kLanePositionOffset + (isBT ? kBTLanePositionDiff : kFXLanePositionDiff) * laneIdx + Vec2::Down(positionStartY - height / 2);
				sourceTexture() // TODO: Chip BT color
					.resized(isBT ? 40 : 82, NoteGraphicsUtils::ChipNoteHeight(yRate))
					.draw(position);
			}
		}
	}
}

void MusicGame::Graphics::ButtonNoteGraphics::drawChipBTNotes(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget) const
{
	drawChipNotesCommon(updateInfo, additiveTarget, true);
}

void MusicGame::Graphics::ButtonNoteGraphics::drawChipFXNotes(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget) const
{
	drawChipNotesCommon(updateInfo, additiveTarget, false);
}

void MusicGame::Graphics::ButtonNoteGraphics::drawLongNotesCommon(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget, bool isBT) const
{
	const kson::ChartData& chartData = *updateInfo.pChartData;
	const double highwayTextureHeight = static_cast<double>(kHighwayTextureSize.y);

	const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);

	for (std::size_t laneIdx = 0; laneIdx < (isBT ? kson::kNumBTLanes : kson::kNumFXLanes); ++laneIdx)
	{
		const auto& lane = isBT ? chartData.note.btLanes[laneIdx] : chartData.note.fxLanes[laneIdx];
		for (const auto& [y, note] : lane)
		{
			if (y + note.length < updateInfo.currentPulse - kson::kResolution)
			{
				continue;
			}

			const double positionStartY = highwayTextureHeight - static_cast<double>(y - updateInfo.currentPulse) * 480 / kson::kResolution;
			if (positionStartY < 0)
			{
				break;
			}

			if (note.length > 0)
			{
				const double positionEndY = highwayTextureHeight - static_cast<double>(y + note.length - updateInfo.currentPulse) * 480 / kson::kResolution;
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
						.resized(width, static_cast<double>(note.length) * 480 / kson::kResolution)
						.draw(position);
				}
			}
		}
	}
}

void MusicGame::Graphics::ButtonNoteGraphics::drawLongBTNotes(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const
{
	drawLongNotesCommon(updateInfo, additiveTarget, invMultiplyTarget, true);
}

void MusicGame::Graphics::ButtonNoteGraphics::drawLongFXNotes(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const
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

void MusicGame::Graphics::ButtonNoteGraphics::draw(const GraphicsUpdateInfo& updateInfo, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const
{
	if (updateInfo.pChartData == nullptr)
	{
		return;
	}

	drawLongFXNotes(updateInfo, additiveTarget, invMultiplyTarget);
	drawLongBTNotes(updateInfo, additiveTarget, invMultiplyTarget);
	drawChipFXNotes(updateInfo, additiveTarget);
	drawChipBTNotes(updateInfo, additiveTarget);
}
