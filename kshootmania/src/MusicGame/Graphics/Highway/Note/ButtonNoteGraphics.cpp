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
		constexpr StringView kLongFXNoteWipeMaskTextureFilename = U"fx_long_wipe_mask.gif";

		constexpr double kLongNoteSourceYDefault = 0.0;
		constexpr double kLongNoteSourceYPressed1 = 8.0;
		constexpr double kLongNoteSourceYPressed2 = 9.0;
		constexpr double kLongNoteSourceYNotPressed = 10.0;

		constexpr double kLongNoteStartTextureY = 1.0;
		constexpr int32 kLongNoteStartTextureHeight = 7;

		constexpr int32 kBTNoteWidth = 40;
		constexpr int32 kFXNoteWidth = 82;

		// ロングノーツの描画パス
		enum class LongNotePass
		{
			kMain, // additive側へノーツを描画
			kWipeMask, // invMultiply側のレーン消失の範囲へロングFXノーツのマスクを描画
			kSub, // invMultiply側へロングBTノーツのinvMultiply用のテクスチャ列を描画
		};

		/// @brief チップノーツの形のマスクを描画する間の色設定
		/// @remark テクスチャの色は白に置き換わり、アルファのみが使用される
		struct ScopedMaskColor
		{
			ScopedColorMul2D colorMul{ 0.0, 1.0 };
			ScopedColorAdd2D colorAdd{ 1.0, 0.0 };
		};

		double PressedLongNoteSourceY(double currentTimeSec)
		{
			return (MathUtils::WrappedFmod(currentTimeSec, 0.1) < 0.05) ? kLongNoteSourceYPressed1 : kLongNoteSourceYPressed2;
		}

		bool ImageExists(StringView filename)
		{
			return FileSystem::Exists(FileSystem::PathAppend(FsUtils::GetResourcePath(U"imgs"), filename));
		}

		Texture CreateLongNoteStartTexture(StringView filename)
		{
			const Image source{ FileSystem::PathAppend(FsUtils::GetResourcePath(U"imgs"), filename) };
			return Texture{ ImageUtils::ReplaceBlackWithTransparent(source.clipped(0, static_cast<int32>(kLongNoteStartTextureY), source.width(), kLongNoteStartTextureHeight)) };
		}

		// レーン消失の範囲で使用するロングノーツのマスクのテクスチャ(画像が無い場合は空)
		// (画像の黒い部分にはマスクがかからないよう透過にする)
		Texture CreateLongNoteWipeMaskTexture(StringView filename, bool isStartTexture)
		{
			if (!ImageExists(filename))
			{
				return Texture{};
			}

			if (isStartTexture)
			{
				return CreateLongNoteStartTexture(filename);
			}

			const Image source{ FileSystem::PathAppend(FsUtils::GetResourcePath(U"imgs"), filename) };
			return Texture{ ImageUtils::ReplaceBlackWithTransparent(source) };
		}

		template <std::size_t N>
		std::array<kson::RelPulse, N> MaxNoteLengths(const std::array<kson::ByPulse<kson::Interval>, N>& lanes)
		{
			std::array<kson::RelPulse, N> maxNoteLengths{};
			for (std::size_t laneIdx = 0; laneIdx < N; ++laneIdx)
			{
				for (const auto& [y, note] : lanes[laneIdx])
				{
					maxNoteLengths[laneIdx] = Max(maxNoteLengths[laneIdx], note.length);
				}
			}
			return maxNoteLengths;
		}

		/// @brief 描画を開始するノーツを取得(描画範囲より下にあるノーツはスキップ)
		/// @param maxNoteLength レーン内の最大ノーツ長(チップノーツのみ描画する場合は0)
		kson::ByPulse<kson::Interval>::const_iterator DrawBeginItr(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse, kson::RelPulse maxNoteLength, const Scroll::HighwayScrollContext& highwayScrollContext)
		{
			if (highwayScrollContext.hasNegativeScrollSpeed())
			{
				// scroll_speedに負の値が含まれている場合は過去のノーツも描画範囲に入る可能性があるため、先頭から描画
				return lane.begin();
			}

			// 終点が描画範囲より下にあるノーツまで戻る(ノーツが重なっていても漏れないよう、長さは最大値で判定)
			auto itr = lane.upper_bound(currentPulse);
			while (itr != lane.begin() && highwayScrollContext.getPositionY(std::prev(itr)->first + maxNoteLength) < kHighwayTextureSize.y)
			{
				--itr;
			}
			return itr;
		}
	}

	void ButtonNoteGraphics::drawChipNotesCommon(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target, bool isBT) const
	{
		const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);

		const std::size_t numLanes = isBT ? kson::kNumBTLanesSZ : kson::kNumFXLanesSZ;

		// レーン消失の範囲ではノーツの背面が暗くならないため、invMultiply側へノーツの形のマスクを描画
		const HighwayWipeRegion wipeRegion = CalcHighwayWipeRegion(viewStatus.camStatus.highwayWipe);
		const int32 numPasses = (wipeRegion.wipeHeight > 0) ? 2 : 1;

		// レンダーターゲットの切り替えはノーツ単位で行うと描画バッチが分断されるため、パス単位でまとめる
		for (int32 pass = 0; pass < numPasses; ++pass)
		{
			const bool isWipeMask = pass == 1;
			const ScopedRenderTarget2D renderTarget(isWipeMask ? target.invMultiplyTexture() : target.additiveTexture());
			Optional<ScopedMaskColor> maskColor;
			if (isWipeMask)
			{
				maskColor.emplace();
			}

			for (std::size_t laneIdx = 0; laneIdx < numLanes; ++laneIdx)
			{
				const auto& lane = isBT ? chartData.note.bt[laneIdx] : chartData.note.fx[laneIdx];
				const double centerSplitShiftX = Camera::CenterSplitShiftX(viewStatus.camStatus.centerSplit) * ((laneIdx >= numLanes / 2) ? 1 : -1);
				const Vec2 offsetPosition = kLanePositionOffset + (isBT ? kBTLanePositionDiff : kFXLanePositionDiff) * static_cast<double>(laneIdx);

				// scroll_speedに負の値が含まれている場合は描画範囲外の判定を変更
				const bool hasNegativeScrollSpeed = highwayScrollContext.hasNegativeScrollSpeed();

				for (auto itr = DrawBeginItr(lane, gameStatus.currentPulse, 0, highwayScrollContext); itr != lane.end(); ++itr)
				{
					const auto& [y, note] = *itr;
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
					const bool hasKeySound = !isBT && m_fxChipKeySoundPulses[kson::kNumBTLanesSZ + laneIdx].contains(y);

					const double yRate = static_cast<double>(kHighwayTextureSize.y - positionStartY) / kHighwayTextureSize.y;
					const int32 height = NoteGraphicsUtils::ChipNoteHeight(yRate);
					const TiledTexture& sourceTexture = isBT ? m_chipBTNoteTexture : hasKeySound ? m_chipFXSENoteTexture : m_chipFXNoteTexture;
					const Vec2 position = offsetPosition + Vec2::Right(centerSplitShiftX) + Vec2::Down(positionStartY - height / 2);
					const int32 colorIndex = isBT ? NoteGraphicsUtils::CalcChipNoteColorIndex(y, chartData.beat, highwayScrollContext.timingCache(), playOption.noteSkin) : 0;
					const TextureRegion region = sourceTexture(0, colorIndex).resized(isBT ? kBTNoteWidth : kFXNoteWidth, height);
					if (isWipeMask)
					{
						DrawInHighwayWipeRegion(region, position, wipeRegion);
					}
					else
					{
						region.draw(position);
					}
				}
			}
		}
	}

	void ButtonNoteGraphics::drawChipBTNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		drawChipNotesCommon(chartData, gameStatus, viewStatus, playOption, highwayScrollContext, target, true);
	}

	void ButtonNoteGraphics::drawChipFXNotes(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		drawChipNotesCommon(chartData, gameStatus, viewStatus, playOption, highwayScrollContext, target, false);
	}

	void ButtonNoteGraphics::drawLongNotesCommon(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target, bool isBT) const
	{
		const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);

		const std::size_t numLanes = isBT ? kson::kNumBTLanesSZ : kson::kNumFXLanesSZ;

		// scroll_speedに負の値が含まれている場合は描画範囲外の判定を変更
		const bool hasNegativeScrollSpeed = highwayScrollContext.hasNegativeScrollSpeed();

		const int32 width = isBT ? kBTNoteWidth : kFXNoteWidth;

		const HighwayWipeRegion wipeRegion = CalcHighwayWipeRegion(viewStatus.camStatus.highwayWipe);

		// レンダーターゲットとブレンドステートの切り替えはノーツ単位で行うと描画バッチが分断されるため、パス単位でまとめる
		for (const LongNotePass pass : { LongNotePass::kMain, LongNotePass::kWipeMask, LongNotePass::kSub })
		{
			if (pass == LongNotePass::kWipeMask && (isBT || wipeRegion.wipeHeight <= 0 || m_longFXNoteWipeMaskTexture.isEmpty()))
			{
				continue;
			}
			if (pass == LongNotePass::kSub && !isBT)
			{
				continue;
			}

			const bool isWipeMask = pass == LongNotePass::kWipeMask;
			const int32 textureColumn = (pass == LongNotePass::kSub) ? kTextureColumnSub : kTextureColumnMain;
			const ScopedRenderTarget2D renderTarget((pass == LongNotePass::kMain) ? target.additiveTexture() : target.invMultiplyTexture());
			const ScopedRenderStates2D blendState(
				(pass == LongNotePass::kSub) ? BlendState::Subtractive
				: (pass == LongNotePass::kMain && isBT) ? BlendState::Additive
				: BlendState::Default2D);
			// 描画(マスクの場合はレーン消失の範囲内の部分のみ)
			const auto fnDraw = [&](const TextureRegion& region, const Vec2& position)
			{
				if (isWipeMask)
				{
					DrawInHighwayWipeRegion(region, position, wipeRegion);
				}
				else
				{
					region.draw(position);
				}
			};

			for (std::size_t laneIdx = 0; laneIdx < numLanes; ++laneIdx)
			{
				const auto& lane = isBT ? chartData.note.bt[laneIdx] : chartData.note.fx[laneIdx];
				const double centerSplitShiftX = Camera::CenterSplitShiftX(viewStatus.camStatus.centerSplit) * ((laneIdx >= numLanes / 2) ? 1 : -1);
				const Vec2 offsetPosition = kLanePositionOffset + (isBT ? kBTLanePositionDiff : kFXLanePositionDiff) * laneIdx;

				const kson::RelPulse maxNoteLength = isBT ? m_btMaxNoteLengths[laneIdx] : m_fxMaxNoteLengths[laneIdx];

				for (auto itr = DrawBeginItr(lane, gameStatus.currentPulse, maxNoteLength, highwayScrollContext); itr != lane.end(); ++itr)
				{
					const auto& [y, note] = *itr;
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
					const Texture& sourceTexture = isWipeMask ? m_longFXNoteWipeMaskTexture : (isBT ? m_longBTNoteTexture : m_longFXNoteTexture);
					// scroll_speedが負の場合、positionStartYとpositionEndYが逆転するため、小さい方を使用
					const Vec2 position = offsetPosition + Vec2::Right(centerSplitShiftX) + Vec2::Down(Min(positionStartY, positionEndY));
					fnDraw(sourceTexture(width * textureColumn, sourceY + kOnePixelTextureSourceOffset, width, kOnePixelTextureSourceSize).resized(width, absHeight), position);

					// 始点テクスチャの描画
					const int32 startTextureY = height >= 0
						? positionStartY - kLongNoteStartTextureHeight
						: positionStartY;
					const Vec2 startPosition = offsetPosition + Vec2::Right(centerSplitShiftX) + Vec2::Down(startTextureY);
					const Texture& startTexture = isWipeMask ? m_longFXNoteWipeMaskStartTexture : (isBT ? m_longBTNoteStartTexture : m_longFXNoteStartTexture);
					fnDraw(startTexture(width * textureColumn, 0, width, kLongNoteStartTextureHeight), startPosition);
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

	ButtonNoteGraphics::ButtonNoteGraphics(const kson::ChartData& chartData, const std::array<HashSet<kson::Pulse>, kNumSRandomLanesSZ>& fxChipKeySoundPulses)
		: m_chipBTNoteTexture(NoteGraphicsUtils::ApplyAlphaToNoteTexture(TextureAsset(kChipBTNoteTextureFilename),
			{
				.column = 9 * kNumTextureColumnsMainSub,
				.sourceSize = { kBTNoteWidth, 14 },
			}))
		, m_longBTNoteTexture(TextureAsset(kLongBTNoteTextureFilename))
		, m_longBTNoteStartTexture(CreateLongNoteStartTexture(kLongBTNoteTextureFilename))
		, m_chipFXNoteTexture(NoteGraphicsUtils::ApplyAlphaToNoteTexture(TextureAsset(kChipFXNoteTextureFilename),
			{
				.column = kNumTextureColumnsMainSub,
				.sourceSize = { kFXNoteWidth, 14 },
			}))
		, m_chipFXSENoteTexture(NoteGraphicsUtils::ApplyAlphaToNoteTexture(TextureAsset(kChipFXSENoteTextureFilename),
			{
				.column = kNumTextureColumnsMainSub,
				.sourceSize = { kFXNoteWidth, 14 },
			}))
		, m_longFXNoteTexture(TextureAsset(kLongFXNoteTextureFilename))
		, m_longFXNoteStartTexture(CreateLongNoteStartTexture(kLongFXNoteTextureFilename))
		, m_longFXNoteWipeMaskTexture(CreateLongNoteWipeMaskTexture(kLongFXNoteWipeMaskTextureFilename, false))
		, m_longFXNoteWipeMaskStartTexture(CreateLongNoteWipeMaskTexture(kLongFXNoteWipeMaskTextureFilename, true))
		, m_fxChipKeySoundPulses(fxChipKeySoundPulses)
		, m_btMaxNoteLengths(MaxNoteLengths(chartData.note.bt))
		, m_fxMaxNoteLengths(MaxNoteLengths(chartData.note.fx))
	{
	}

	void ButtonNoteGraphics::draw(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const PlayOption& playOption, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		drawLongFXNotes(chartData, gameStatus, viewStatus, highwayScrollContext, target);
		drawLongBTNotes(chartData, gameStatus, viewStatus, highwayScrollContext, target);
		drawChipFXNotes(chartData, gameStatus, viewStatus, playOption, highwayScrollContext, target);
		drawChipBTNotes(chartData, gameStatus, viewStatus, playOption, highwayScrollContext, target);
	}
}
