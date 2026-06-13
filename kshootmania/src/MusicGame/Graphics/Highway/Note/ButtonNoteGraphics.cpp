#include "ButtonNoteGraphics.hpp"
#include "NoteGraphicsUtils.hpp"
#include "MusicGame/Graphics/GraphicsDefines.hpp"
#include "MusicGame/Camera/CameraMath.hpp"
#include "Graphics/ImageUtils.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kChipBTNoteTextureFilename = U"bt_chip.gif";
		constexpr StringView kLongBTNoteTextureFilename = U"bt_long.gif";

		constexpr StringView kChipFXNoteTextureFilename = U"fx_chip.gif";
		constexpr StringView kChipFXSENoteTextureFilename = U"fx_chip_se.gif";
		constexpr StringView kLongFXNoteTextureFilename = U"fx_long.gif";

		constexpr double kLongNoteSourceYDefault = 0.0;
		constexpr double kLongNoteSourceYPressed1 = 8.0;
		constexpr double kLongNoteSourceYPressed2 = 9.0;
		constexpr double kLongNoteSourceYNotPressed = 10.0;

		constexpr double kLongNoteStartTextureY = 1.0;
		constexpr int32 kLongNoteStartTextureHeight = 7;

		double PressedLongNoteSourceY(double currentTimeSec)
		{
			return (MathUtils::WrappedFmod(currentTimeSec, 0.1) < 0.05) ? kLongNoteSourceYPressed1 : kLongNoteSourceYPressed2;
		}

		Texture CreateLongNoteStartTexture(StringView filename)
		{
			const Image source{ FileSystem::PathAppend(FsUtils::GetResourcePath(U"imgs"), filename) };
			return Texture{ ImageUtils::ReplaceBlackWithTransparent(source.clipped(0, static_cast<int32>(kLongNoteStartTextureY), source.width(), kLongNoteStartTextureHeight)) };
		}
	}

	void ButtonNoteGraphics::drawChipNotesCommon(const kson::ChartData& chartData, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target, bool isBT) const
	{
		const ScopedRenderTarget2D renderTarget(target.additiveTexture());
		const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);

		const std::size_t numLanes = isBT ? kson::kNumBTLanesSZ : kson::kNumFXLanesSZ;
		for (std::size_t laneIdx = 0; laneIdx < numLanes; ++laneIdx)
		{
			const auto& lane = isBT ? chartData.note.bt[laneIdx] : chartData.note.fx[laneIdx];
			const double centerSplitShiftX = Camera::CenterSplitShiftX(viewStatus.camStatus.centerSplit) * ((laneIdx >= numLanes / 2) ? 1 : -1);
			const Vec2 offsetPosition = kLanePositionOffset + (isBT ? kBTLanePositionDiff : kFXLanePositionDiff) * static_cast<double>(laneIdx);

			// scroll_speedに負の値が含まれている場合は描画範囲外の判定を変更
			const bool hasNegativeScrollSpeed = highwayScrollContext.hasNegativeScrollSpeed();

			for (const auto& [y, note] : lane)
			{
				const int32 positionStartY = highwayScrollContext.getPositionY(y);

				// 描画範囲外チェック
				if (positionStartY < 0 || positionStartY >= kHighwayTextureSize.y)
				{
					if (!hasNegativeScrollSpeed && positionStartY < 0)
					{
						// scroll_speedに負の値がなく、ノーツが上にある場合はループを抜ける
						break;
					}
					continue;
				}

				if (note.length != 0)
				{
					// ここではチップノーツ以外は描画しない
					continue;
				}

				// 音ありFX描画の可否
				// TODO: キー音有無は譜面ロード時にノーツ毎に事前判定しておく
				bool hasKeySound = false;
				const auto& chipEvent = chartData.audio.keySound.fx.chipEvent;
				for (const auto& [filename, lanes] : chipEvent)
				{
					if (laneIdx >= lanes.size())
					{
						continue;
					}

					if (lanes[laneIdx].contains(y))
					{
						hasKeySound = true;
						break;
					}
				}

				const double yRate = static_cast<double>(kHighwayTextureSize.y - positionStartY) / kHighwayTextureSize.y;
				const int32 height = NoteGraphicsUtils::ChipNoteHeight(yRate);
				const TiledTexture& sourceTexture = isBT ? m_chipBTNoteTexture : hasKeySound ? m_chipFXSENoteTexture : m_chipFXNoteTexture;
				const Vec2 position = offsetPosition + Vec2::Right(centerSplitShiftX) + Vec2::Down(positionStartY - height / 2);
				const int32 colorIndex = isBT ? NoteGraphicsUtils::CalcChipNoteColorIndex(y, chartData.beat, playOption.noteSkin) : 0;
				sourceTexture(0, colorIndex)
					.resized(isBT ? 40 : 82, height)
					.draw(position);
			}
		}
	}

	void ButtonNoteGraphics::drawChipBTNotes(const kson::ChartData& chartData, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		drawChipNotesCommon(chartData, viewStatus, playOption, highwayScrollContext, target, true);
	}

	void ButtonNoteGraphics::drawChipFXNotes(const kson::ChartData& chartData, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		drawChipNotesCommon(chartData, viewStatus, playOption, highwayScrollContext, target, false);
	}

	void ButtonNoteGraphics::drawLongNotesCommon(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target, bool isBT) const
	{
		const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);

		const std::size_t numLanes = isBT ? kson::kNumBTLanesSZ : kson::kNumFXLanesSZ;
		for (std::size_t laneIdx = 0; laneIdx < numLanes; ++laneIdx)
		{
			const auto& lane = isBT ? chartData.note.bt[laneIdx] : chartData.note.fx[laneIdx];
			const double centerSplitShiftX = Camera::CenterSplitShiftX(viewStatus.camStatus.centerSplit) * ((laneIdx >= numLanes / 2) ? 1 : -1);
			const Vec2 offsetPosition = kLanePositionOffset + (isBT ? kBTLanePositionDiff : kFXLanePositionDiff) * laneIdx;

			// scroll_speedに負の値が含まれている場合は描画範囲外の判定を変更
			const bool hasNegativeScrollSpeed = highwayScrollContext.hasNegativeScrollSpeed();

			for (const auto& [y, note] : lane)
			{
				const int32 positionStartY = highwayScrollContext.getPositionY(y);
				if (!hasNegativeScrollSpeed && positionStartY < 0)
				{
					// scroll_speedに負の値がない場合は、positionStartY < 0でループを抜ける
					break;
				}

				const int32 positionEndY = note.length == 0 ? positionStartY : highwayScrollContext.getPositionY(y + note.length);

				// ノーツ全体が描画範囲外の場合はスキップ
				if (!highwayScrollContext.isPulseRangeInDrawRange(y, y + note.length))
				{
					if (!hasNegativeScrollSpeed && Max(positionStartY, positionEndY) < 0)
					{
						// scroll_speedに負の値がなく、ノーツ全体が上にある場合はループを抜ける
						break;
					}
					continue;
				}

				if (note.length <= 0)
				{
					// ここではロングノーツ以外は描画しない
					continue;
				}

				const int32 height = positionStartY - positionEndY;
				if (height == 0)
				{
					// 高さが0の場合は描画しない
					continue;
				}

				// scroll_speedが負の場合、heightが負になる可能性がある
				const int32 absHeight = Abs(height);
				if (absHeight <= 0)
				{
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
					const Texture& sourceTexture = isBT ? m_longBTNoteTexture : m_longFXNoteTexture;
					const int32 width = isBT ? 40 : 82;
					// scroll_speedが負の場合、positionStartYとpositionEndYが逆転するため、小さい方を使用
					const Vec2 position = offsetPosition + Vec2::Right(centerSplitShiftX) + Vec2::Down(Min(positionStartY, positionEndY));
					sourceTexture(width * i, sourceY + kOnePixelTextureSourceOffset, width, kOnePixelTextureSourceSize)
						.resized(width, absHeight)
						.draw(position);

					// 始点テクスチャの描画
					const int32 startTextureY = height >= 0
						? positionStartY - kLongNoteStartTextureHeight
						: positionStartY;
					const Vec2 startPosition = offsetPosition + Vec2::Right(centerSplitShiftX) + Vec2::Down(startTextureY);
					const Texture& startTexture = isBT ? m_longBTNoteStartTexture : m_longFXNoteStartTexture;
					startTexture(width * i, 0, width, kLongNoteStartTextureHeight)
						.draw(startPosition);
				}
			}
		}
	}

	void ButtonNoteGraphics::drawLongBTNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		drawLongNotesCommon(chartData, gameStatus, viewStatus, highwayScrollContext, target, true);
	}

	void ButtonNoteGraphics::drawLongFXNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		drawLongNotesCommon(chartData, gameStatus, viewStatus, highwayScrollContext, target, false);
	}

	ButtonNoteGraphics::ButtonNoteGraphics()
		: m_chipBTNoteTexture(NoteGraphicsUtils::ApplyAlphaToNoteTexture(TextureAsset(kChipBTNoteTextureFilename),
			{
				.column = 9 * kNumTextureColumnsMainSub,
				.sourceSize = { 40, 14 },
			}))
		, m_longBTNoteTexture(TextureAsset(kLongBTNoteTextureFilename))
		, m_longBTNoteStartTexture(CreateLongNoteStartTexture(kLongBTNoteTextureFilename))
		, m_chipFXNoteTexture(NoteGraphicsUtils::ApplyAlphaToNoteTexture(TextureAsset(kChipFXNoteTextureFilename),
			{
				.column = kNumTextureColumnsMainSub,
				.sourceSize = { 82, 14 },
			}))
		, m_chipFXSENoteTexture(NoteGraphicsUtils::ApplyAlphaToNoteTexture(TextureAsset(kChipFXSENoteTextureFilename),
			{
				.column = kNumTextureColumnsMainSub,
				.sourceSize = { 82, 14 },
			}))
		, m_longFXNoteTexture(TextureAsset(kLongFXNoteTextureFilename))
		, m_longFXNoteStartTexture(CreateLongNoteStartTexture(kLongFXNoteTextureFilename))
	{
	}

	void ButtonNoteGraphics::draw(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		drawLongFXNotes(chartData, gameStatus, viewStatus, highwayScrollContext, target);
		drawLongBTNotes(chartData, gameStatus, viewStatus, highwayScrollContext, target);
		drawChipFXNotes(chartData, viewStatus, playOption, highwayScrollContext, target);
		drawChipBTNotes(chartData, viewStatus, playOption, highwayScrollContext, target);
	}
}
