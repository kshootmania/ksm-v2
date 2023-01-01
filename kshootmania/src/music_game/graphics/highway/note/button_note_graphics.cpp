#include "button_note_graphics.hpp"
#include "note_graphics_utils.hpp"
#include "music_game/graphics/graphics_defines.hpp"

namespace MusicGame::Graphics
{
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

	void ButtonNoteGraphics::drawChipNotesCommon(const kson::ChartData& chartData, const GameStatus& gameStatus, const HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target, bool isBT) const
	{
		const ScopedRenderTarget2D renderTarget(target.additiveTexture());
		const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);

		for (std::size_t laneIdx = 0; laneIdx < (isBT ? kson::kNumBTLanesSZ : kson::kNumFXLanesSZ); ++laneIdx)
		{
			const auto& lane = isBT ? chartData.note.bt[laneIdx] : chartData.note.fx[laneIdx];
			const Vec2 offsetPosition = kLanePositionOffset + (isBT ? kBTLanePositionDiff : kFXLanePositionDiff) * static_cast<double>(laneIdx);
			for (const auto& [y, note] : lane)
			{
				const int32 positionStartY = highwayScrollContext.getPositionY(y);
				if (positionStartY < 0)
				{
					break;
				}
				if (positionStartY >= kHighwayTextureSize.y)
				{
					continue;
				}

				if (note.length != 0)
				{
					// ここではチップノーツ以外は描画しない
					continue;
				}

				const double yRate = static_cast<double>(kHighwayTextureSize.y - positionStartY) / kHighwayTextureSize.y;
				const int32 height = NoteGraphicsUtils::ChipNoteHeight(yRate);
				const TiledTexture& sourceTexture = isBT ? m_chipBTNoteTexture : m_chipFXNoteTexture;
				const Vec2 position = offsetPosition + Vec2::Down(positionStartY - height / 2);
				sourceTexture() // TODO: Chip BT color
					.resized(isBT ? 40 : 82, height)
					.draw(position);
			}
		}
	}

	void ButtonNoteGraphics::drawChipBTNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		drawChipNotesCommon(chartData, gameStatus, highwayScrollContext, target, true);
	}

	void ButtonNoteGraphics::drawChipFXNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		drawChipNotesCommon(chartData, gameStatus, highwayScrollContext, target, false);
	}

	void ButtonNoteGraphics::drawLongNotesCommon(const kson::ChartData& chartData, const GameStatus& gameStatus, const HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target, bool isBT) const
	{
		const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);

		for (std::size_t laneIdx = 0; laneIdx < (isBT ? kson::kNumBTLanesSZ : kson::kNumFXLanesSZ); ++laneIdx)
		{
			const auto& lane = isBT ? chartData.note.bt[laneIdx] : chartData.note.fx[laneIdx];
			const Vec2 offsetPosition = kLanePositionOffset + (isBT ? kBTLanePositionDiff : kFXLanePositionDiff) * laneIdx;
			for (const auto& [y, note] : lane)
			{
				const int32 positionStartY = highwayScrollContext.getPositionY(y);
				if (positionStartY < 0)
				{
					break;
				}

				const int32 positionEndY = note.length == 0 ? positionStartY : highwayScrollContext.getPositionY(y + note.length);
				if (positionEndY >= kHighwayTextureSize.y)
				{
					continue;
				}

				if (note.length <= 0)
				{
					// ここではロングノーツ以外は描画しない
					continue;
				}

				const int32 height = positionStartY - positionEndY;
				if (height <= 0)
				{
					// TODO: scroll_speedに逆再生を実装すると負の高さを考慮する必要が出てくる
					continue;
				}

				const int32 numColumns = isBT ? kNumTextureColumnsMainSub : 1; // ロングBTノーツの場合はinvMultiply用のテクスチャ列が追加で存在する
				for (int32 i = 0; i < numColumns; ++i)
				{
					const ScopedRenderTarget2D renderTarget((i == 0) ? target.additiveTexture() : target.invMultiplyTexture());
					const ScopedRenderStates2D blendState((i == 0) ? (isBT ? BlendState::Additive : BlendState::Default2D) : BlendState::Subtractive);
					const ButtonLaneStatus& laneStatus = isBT ? gameStatus.btLaneStatus[laneIdx] : gameStatus.fxLaneStatus[laneIdx];
					double sourceY;
					if (laneStatus.currentLongNotePulse == y)
					{
						// 現在判定対象の押下中のロングノーツ
						sourceY = PressedLongNoteSourceY(gameStatus.currentTimeSec);
					}
					else if (y <= gameStatus.currentPulse && gameStatus.currentPulse < y + note.length)
					{
						// 現在判定対象だが押していないロングノーツ
						sourceY = kLongNoteSourceYNotPressed;
					}
					else
					{
						// 現在判定対象でないロングノーツ
						sourceY = kLongNoteSourceYDefault;
					}
					// TODO: 始点テクスチャの描画
					const Texture& sourceTexture = isBT ? m_longBTNoteTexture : m_longFXNoteTexture;
					const int32 width = isBT ? 40 : 82;
					const Vec2 position = offsetPosition + Vec2::Down(positionEndY);
					sourceTexture(width * i, sourceY + kOnePixelTextureSourceOffset, width, kOnePixelTextureSourceSize)
						.resized(width, height)
						.draw(position);
				}
			}
		}
	}

	void ButtonNoteGraphics::drawLongBTNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		drawLongNotesCommon(chartData, gameStatus, highwayScrollContext, target, true);
	}

	void ButtonNoteGraphics::drawLongFXNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		drawLongNotesCommon(chartData, gameStatus, highwayScrollContext, target, false);
	}

	ButtonNoteGraphics::ButtonNoteGraphics()
		: m_chipBTNoteTexture(NoteGraphicsUtils::ApplyAlphaToNoteTexture(TextureAsset(kChipBTNoteTextureFilename),
			{
				.column = 9 * kNumTextureColumnsMainSub,
				.sourceSize = { 40, 14 },
			}))
			, m_longBTNoteTexture(TextureAsset(kLongBTNoteTextureFilename))
		, m_chipFXNoteTexture(NoteGraphicsUtils::ApplyAlphaToNoteTexture(TextureAsset(kChipFXNoteTextureFilename),
			{
				.column = kNumTextureColumnsMainSub,
				.sourceSize = { 82, 14 },
			}))
		, m_longFXNoteTexture(TextureAsset(kLongFXNoteTextureFilename))
	{
	}

	void ButtonNoteGraphics::draw(const kson::ChartData& chartData, const GameStatus& gameStatus, const HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		drawLongFXNotes(chartData, gameStatus, highwayScrollContext, target);
		drawLongBTNotes(chartData, gameStatus, highwayScrollContext, target);
		drawChipFXNotes(chartData, gameStatus, highwayScrollContext, target);
		drawChipBTNotes(chartData, gameStatus, highwayScrollContext, target);
	}
}
