#include "LaserNoteGraphics.hpp"
#include "NoteGraphicsUtils.hpp"
#include "MusicGame/Graphics/GraphicsDefines.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kLaserNoteTextureFilename = U"laser.gif";
		constexpr StringView kLaserNoteMaskTextureFilename = U"laser_mask.gif";
		constexpr StringView kLaserNoteLeftStartTextureFilename = U"laserl_0.gif";
		constexpr StringView kLaserNoteRightStartTextureFilename = U"laserr_0.gif";

		constexpr Size kLaserTextureSize = { 48, 48 };
		constexpr int32 kLaserLineWidth = kLaserTextureSize.x;
		constexpr int32 kLaserSlamHeightMax = kLaserTextureSize.y;
		constexpr kson::RelPulse kLaserSlamHeightPulse = kson::kResolution4 / 32; // 32分長
		constexpr int32 kLaserTailHeightMax = 80;
		constexpr kson::RelPulse kLaserTailHeightPulse = kson::kResolution4 / 16; // 16分長
		constexpr Size kLaserStartTextureSize = { 44, 200 };
		constexpr int32 kLaserShiftY = -6;

		constexpr double kLaserCriticalBlinkIntervalSec = 0.12;

		enum class JudgmentStatus
		{
			kNormal,
			kCritical,
			kError,
		};

		constexpr Quad LaserLineQuad(const Vec2& positionStart, const Vec2& positionEnd)
		{
			return {
				positionStart + Vec2{ kLaserLineWidth / 2, 0.0 },
				positionStart + Vec2{ -kLaserLineWidth / 2, 0.0 },
				positionEnd + Vec2{ -kLaserLineWidth / 2, 0.0 },
				positionEnd + Vec2{ kLaserLineWidth / 2, 0.0 }
			};
		}

		constexpr double LaserPointX(double v, int32 xScale)
		{
			return (v * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2) * xScale;
		}

		void DrawLaserLine(int32 laneIdx, int32 positionY, const kson::GraphPoint& point, int32 slamHeight, int32 nextPositionY, const kson::GraphPoint& nextPoint, const Texture& laserNoteTexture, int32 laserNoteTextureRow, int32 xScale, double startXShift)
		{
			// 現在の点が直角の場合は後続のLASERを高さ分後ろにずらす
			const bool isSlam = point.v.v != point.v.vf;
			const int32 currentOffset = isSlam ? -slamHeight : 0;

			const Vec2 positionStart = {
				LaserPointX(point.v.vf, xScale) + startXShift,
				positionY + currentOffset
			};

			const Vec2 positionEnd = {
				LaserPointX(nextPoint.v.v, xScale),
				nextPositionY
			};

			const Quad quad = LaserLineQuad(positionStart, positionEnd);
			quad(laserNoteTexture(kLaserTextureSize.x * laneIdx, kLaserTextureSize.y * laserNoteTextureRow + kLaserTextureSize.y - 1 + kOnePixelTextureSourceOffset, kLaserTextureSize.x, kOnePixelTextureSourceSize)).draw();
		}

		/// @brief 直角レーザーの横線の高さを計算(48pxと32分長の高さの小さい方)
		/// @remark scroll_speedが負の場合は負の値を返す
		int32 CalcLaserSlamHeight(kson::Pulse pulse, const Scroll::HighwayScrollContext& highwayScrollContext)
		{
			const int32 slamHeightFromPulse = highwayScrollContext.relPulseToPixelHeight(pulse, kLaserSlamHeightPulse);
			const int32 sign = Sign(slamHeightFromPulse);
			return sign * Min(kLaserSlamHeightMax, Abs(slamHeightFromPulse));
		}

		/// @brief 直角レーザーのtailの高さを計算(80pxと16分長の高さの小さい方)
		/// @remark scroll_speedが負の場合は負の値を返す
		int32 CalcLaserTailHeight(kson::Pulse pulse, const Scroll::HighwayScrollContext& highwayScrollContext)
		{
			const int32 tailHeightFromPulse = highwayScrollContext.relPulseToPixelHeight(pulse, kLaserTailHeightPulse);
			const int32 sign = Sign(tailHeightFromPulse);
			return sign * Min(kLaserTailHeightMax, Abs(tailHeightFromPulse));
		}

		Quad LaserSlamLineQuad(const Vec2& positionStart, const Vec2& positionEnd, int32 slamHeight, double topShiftX)
		{
			if (Abs(positionEnd.x - positionStart.x) <= kLaserLineWidth)
			{
				// 左右が近すぎて直角レーザーの横線が描画できない場合はゼロを返す
				return { Vec2::Zero(), Vec2::Zero(), Vec2::Zero(), Vec2::Zero() };
			}

			const int32 diffXSign = Sign(positionEnd.x - positionStart.x);

			if (slamHeight > 0)
			{
				// scroll_speedが正の場合、奥方向(上)に太さを持つ
				return {
					positionStart + Vec2{ diffXSign * kLaserLineWidth / 2, -slamHeight },
					positionEnd + Vec2{ -diffXSign * kLaserLineWidth / 2 + topShiftX, -slamHeight },
					positionEnd + Vec2{ -diffXSign * kLaserLineWidth / 2, 0 },
					positionStart + Vec2{ diffXSign * kLaserLineWidth / 2, 0 }
				};
			}
			else
			{
				// scroll_speedが負の場合、手前方向(下)に太さを持つ
				return {
					positionStart + Vec2{ diffXSign * kLaserLineWidth / 2, 0 },
					positionEnd + Vec2{ -diffXSign * kLaserLineWidth / 2, 0 },
					positionEnd + Vec2{ -diffXSign * kLaserLineWidth / 2 + topShiftX, -slamHeight },
					positionStart + Vec2{ diffXSign * kLaserLineWidth / 2, -slamHeight }
				};
			}
		}

		void DrawLaserSlam(int32 laneIdx, int32 positionY, const kson::GraphPoint& point, int32 slamHeight, const Texture& laserNoteTexture, int32 laserNoteTextureRow, int32 xScale, double topShiftX)
		{
			const Vec2 positionStart = {
				LaserPointX(point.v.v, xScale),
				positionY
			};

			const Vec2 positionEnd = {
				LaserPointX(point.v.vf, xScale),
				positionY
			};

			// 直角レーザーの角のテクスチャを描画
			const bool isLeftToRight = (point.v.v < point.v.vf);
			const int32 yOffset = -slamHeight / 2;
			const double yScale = static_cast<double>(Abs(slamHeight)) / kLaserTextureSize.y;

			// 始点側(v)の角テクスチャ
			if (slamHeight > 0)
			{
				// scroll_speedが正の場合、奥方向(上)に太さを持つ
				laserNoteTexture(kLaserTextureSize.x * laneIdx, kLaserTextureSize.y * laserNoteTextureRow, kLaserTextureSize).mirrored(isLeftToRight).scaled(1.0, yScale).drawAt(positionStart + Vec2{ 0, yOffset });
			}
			else
			{
				// scroll_speedが負の場合、手前方向(下)に太さを持つ(上下反転)
				laserNoteTexture(kLaserTextureSize.x * laneIdx, kLaserTextureSize.y * laserNoteTextureRow, kLaserTextureSize).mirrored(isLeftToRight).flipped().scaled(1.0, yScale).drawAt(positionStart + Vec2{ 0, yOffset });
			}

			// 終点側(vf)の角テクスチャ(直角レーザーの曲線描画時の変形のためにQuadで描画)
			{
				const double endX = positionEnd.x;
				const Vec2 junctionLeft{ endX - kLaserLineWidth / 2 + topShiftX, positionY - slamHeight };
				const Vec2 junctionRight{ endX + kLaserLineWidth / 2 + topShiftX, positionY - slamHeight };
				const Vec2 barLeft{ endX - kLaserLineWidth / 2, positionY };
				const Vec2 barRight{ endX + kLaserLineWidth / 2, positionY };

				const auto textureRegion = laserNoteTexture(kLaserTextureSize.x * laneIdx, kLaserTextureSize.y * laserNoteTextureRow, kLaserTextureSize);

				if (isLeftToRight)
				{
					Quad{ barLeft, barRight, junctionRight, junctionLeft }(textureRegion).draw();
				}
				else
				{
					Quad{ barRight, barLeft, junctionLeft, junctionRight }(textureRegion).draw();
				}
			}

			// 直角レーザーの横線を描画
			const Quad quad = LaserSlamLineQuad(positionStart, positionEnd, slamHeight, topShiftX);
			quad(laserNoteTexture(kLaserTextureSize.x * laneIdx + kOnePixelTextureSourceOffset, kLaserTextureSize.y * laserNoteTextureRow, kOnePixelTextureSourceSize, kLaserTextureSize.y)).draw();
		}

		void DrawLaserSlamTail(int32 laneIdx, int32 positionY, const kson::GraphPoint& point, int32 slamHeight, int32 tailHeight, const Texture& laserNoteTexture, int32 laserNoteTextureRow, int32 xScale)
		{
			const Vec2 positionStart = {
				LaserPointX(point.v.vf, xScale),
				positionY
			};

			// scroll_speedが正の場合は上方向、負の場合は下方向にtailを描画
			const Quad quad = LaserLineQuad(positionStart + Vec2{ 0.0, -slamHeight }, positionStart + Vec2{ 0.0, -slamHeight - tailHeight });
			quad(laserNoteTexture(kLaserTextureSize.x * laneIdx, kLaserTextureSize.y * laserNoteTextureRow + kLaserTextureSize.y - 1 + kOnePixelTextureSourceOffset, kLaserTextureSize.x, kOnePixelTextureSourceSize)).draw();
		}

		JudgmentStatus GetLaserSectionJudgmentStatus(const LaserLaneStatus& laneStatus, kson::Pulse laserSectionY)
		{
			if (laneStatus.currentLaserSectionPulse != laserSectionY)
			{
				// 現在判定対象になっていなければNormal
				return JudgmentStatus::kNormal;
			}

			if (laneStatus.isCursorInCriticalJudgmentRange())
			{
				// カーソルがクリティカル判定の範囲内であればCritical
				return JudgmentStatus::kCritical;
			}

			// 判定が外れていればError
			return JudgmentStatus::kError;
		}

		void DrawLaserSection(int32 laneIdx, kson::Pulse y, const kson::LaserSection& laserSection, const HashSet<kson::Pulse>& curvedPulses, const Scroll::HighwayScrollContext& highwayScrollContext, const RenderTexture& target, const Texture& laserNoteTexture, int32 laserNoteTextureRow, const TextureRegion& laserStartTexture)
		{
			const ScopedRenderTarget2D renderTarget(target);

			// はみ出しLASERの場合は描画領域を横幅2倍として扱う
			const bool wide = laserSection.wide();
			const int32 xScale = wide ? kLaserXScaleWide : kLaserXScaleNormal;
			const Transformer2D transformer(Mat3x2::Translate(wide ? kLaserPositionOffsetWide : kLaserPositionOffsetNormal));

			// scroll_speedに負の値が含まれている場合は過去の点も描画範囲に入る可能性がある
			const bool hasNegativeScrollSpeed = highwayScrollContext.hasNegativeScrollSpeed();

			// LASERセクション内の各点をもとに描画
			for (auto itr = laserSection.v.begin(); itr != laserSection.v.end(); ++itr)
			{
				const auto& [ry, point] = *itr;

				const int32 positionY = highwayScrollContext.getPositionY(y + ry) + kLaserShiftY;

				// この点のPulse位置でのscroll_speedが正かを取得
				const bool isScrollSpeedPositive = highwayScrollContext.isScrollSpeedPositiveAt(y + ry);

				// レーザー開始テクスチャを描画
				if (itr == laserSection.v.begin())
				{
					// 開始テクスチャの描画範囲を計算
					const int32 startTextureMinY = isScrollSpeedPositive ? positionY : positionY - kLaserStartTextureSize.y;
					const int32 startTextureMaxY = isScrollSpeedPositive ? positionY + kLaserStartTextureSize.y : positionY;
					const bool isStartTextureInRange = startTextureMaxY >= 0 && startTextureMinY < kHighwayTextureSize.y;

					if (isStartTextureInRange)
					{
						const Vec2 positionStart = {
							LaserPointX(point.v.v, xScale),
							positionY
						};

						// scroll_speedが正の場合は下方向、負の場合は上方向にテクスチャを配置
						if (isScrollSpeedPositive)
						{
							laserStartTexture.draw(Arg::topCenter = positionStart);
						}
						else
						{
							laserStartTexture.flipped().draw(Arg::bottomCenter = positionStart);
						}
					}
				}

				// 終点スプライトの傾き計算(直角かつ曲線の場合)
				double topShiftX = 0.0;

				// 直角レーザーを描画
				if (point.v.v != point.v.vf)
				{
					const int32 slamHeight = CalcLaserSlamHeight(y + ry, highwayScrollContext);

					// 展開前データに曲線がある場合、slam分厚さの外にある最初の点を参照してtopShiftXを計算
					if (slamHeight != 0 && curvedPulses.contains(y + ry))
					{
						const int32 slamTopY = positionY - slamHeight;
						for (auto refItr = std::next(itr); refItr != laserSection.v.end(); ++refItr)
						{
							const auto& [refRy, refPoint] = *refItr;
							const int32 refPositionY = highwayScrollContext.getPositionY(y + refRy) + kLaserShiftY;

							// slam分厚さ以内の点は飛ばす(境界上の点も含む)
							if ((slamHeight > 0 && refPositionY >= slamTopY) || (slamHeight < 0 && refPositionY <= slamTopY))
							{
								continue;
							}

							const double endPointX = LaserPointX(point.v.vf, xScale);
							const double refPointX = LaserPointX(refPoint.v.v, xScale);
							topShiftX = (refPointX - endPointX) * static_cast<double>(slamHeight) / (positionY - refPositionY);
							break;
						}
					}

					// 直角レーザーの横線は厚さ(slamHeight)を持つため、その範囲をチェック
					const int32 slamMinY = Min(positionY - slamHeight, positionY);
					const int32 slamMaxY = Max(positionY - slamHeight, positionY);
					const bool isSlamInRange = slamMaxY >= 0 && slamMinY < kHighwayTextureSize.y;

					if (isSlamInRange)
					{
						DrawLaserSlam(laneIdx, positionY, point, slamHeight, laserNoteTexture, laserNoteTextureRow, xScale, topShiftX);
					}

					// slam分厚さ以内の次の点を飛ばす
					const int32 slamTopY = positionY - slamHeight;
					while (std::next(itr) != laserSection.v.end())
					{
						const int32 peekPositionY = highwayScrollContext.getPositionY(y + std::next(itr)->first) + kLaserShiftY;
						if ((slamHeight > 0 && peekPositionY >= slamTopY) || (slamHeight < 0 && peekPositionY <= slamTopY))
						{
							++itr;
						}
						else
						{
							break;
						}
					}
				}

				// レーザー終端の点の場合は線を描画しない
				const auto nextItr = std::next(itr);
				if (nextItr == laserSection.v.end())
				{
					// 終端が直角の場合は終端を伸ばす
					if (point.v.v != point.v.vf)
					{
						const int32 slamHeight = CalcLaserSlamHeight(y + ry, highwayScrollContext);
						const int32 tailHeight = CalcLaserTailHeight(y + ry, highwayScrollContext);

						// tailの描画範囲を計算
						const int32 tailMinY = Min(positionY - slamHeight - tailHeight, positionY - slamHeight);
						const int32 tailMaxY = Max(positionY - slamHeight - tailHeight, positionY - slamHeight);
						const bool isTailInRange = tailMaxY >= 0 && tailMinY < kHighwayTextureSize.y;

						if (isTailInRange)
						{
							DrawLaserSlamTail(laneIdx, positionY, point, slamHeight, tailHeight, laserNoteTexture, laserNoteTextureRow, xScale);
						}
					}

					break;
				}

				// レーザーの2つの点をもとに線を描画
				{
					const auto& [nextRy, nextPoint] = *nextItr;
					const int32 nextPositionY = highwayScrollContext.getPositionY(y + nextRy) + kLaserShiftY;

					// 現在の点が直角の場合は後続のLASERを高さ分後ろにずらす
					const bool isSlam = point.v.v != point.v.vf;
					const int32 slamHeight = isSlam ? CalcLaserSlamHeight(y + ry, highwayScrollContext) : 0;

					// scroll_speedが負の場合、始点と終点が逆転する可能性があるため両方チェック
					const int32 minY = Min(positionY, nextPositionY);
					const int32 maxY = Max(positionY, nextPositionY);

					// 線の両端点が完全に描画範囲外の場合はスキップ
					if (maxY < 0 || minY >= kHighwayTextureSize.y)
					{
						// scroll_speedに負の値がなく、線全体が上にある場合は以降も描画対象外
						if (!hasNegativeScrollSpeed && maxY < 0)
						{
							return;
						}
						continue;
					}

					DrawLaserLine(laneIdx, positionY, point, slamHeight, nextPositionY, nextPoint, laserNoteTexture, laserNoteTextureRow, xScale, topShiftX);
				}
			}
		}

		int32 LaserTextureRow(JudgmentStatus judgmentStatus, double currentTimeSec)
		{
			switch (judgmentStatus)
			{
			case JudgmentStatus::kCritical:
				return MathUtils::WrappedFmod(currentTimeSec, kLaserCriticalBlinkIntervalSec) < kLaserCriticalBlinkIntervalSec / 2 ? 2 : 1;

			case JudgmentStatus::kError:
				return 3;

			case JudgmentStatus::kNormal:
			default:
				return 0;
			}
		}
	}

	LaserNoteGraphics::LaserNoteGraphics()
		: m_laserNoteTexture(TextureAsset(kLaserNoteTextureFilename))
		, m_laserNoteMaskTexture(TextureAsset(kLaserNoteMaskTextureFilename))
		, m_laserNoteStartTextures{
			TiledTexture(
				kLaserNoteLeftStartTextureFilename,
				{
					.column = kNumTextureColumnsMainSub,
					.sourceSize = kLaserStartTextureSize,
				}),
			TiledTexture(
				kLaserNoteRightStartTextureFilename,
				{
					.column = kNumTextureColumnsMainSub,
					.sourceSize = kLaserStartTextureSize,
				}) }
	{
	}

	void LaserNoteGraphics::draw(const kson::ChartData& chartData, const std::array<HashSet<kson::Pulse>, kson::kNumLaserLanesSZ>& laserCurvedPulses, const PlayOption& playOption, const GameStatus& gameStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const
	{
		const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);
		const ScopedRenderStates2D renderState(BlendState::Additive);

		// LASERノーツを描画
		for (int32 laneIdx = 0; laneIdx < kson::kNumLaserLanes; ++laneIdx) // 座標計算で結局int32にする必要があるのでここではsize_t不使用
		{
			const auto& lane = chartData.note.laser[laneIdx];
			const auto& curvedPulses = laserCurvedPulses[laneIdx];
			const auto& laneStatus = gameStatus.laserLaneStatus[laneIdx];

			// scroll_speedに負の値が含まれている場合は過去のノーツも描画範囲に入る可能性があるため、先頭から走査
			const bool hasNegativeScrollSpeed = highwayScrollContext.hasNegativeScrollSpeed();
			auto itr = hasNegativeScrollSpeed ? lane.begin() : kson::ValueItrAt(lane, gameStatus.currentPulse);

			for (; itr != lane.end(); ++itr)
			{
				const auto& [y, laserSection] = *itr;

				const kson::RelPulse lengthRy = laserSection.v.rbegin()->first;
				const int32 sectionStartPositionY = highwayScrollContext.getPositionY(y) + kLaserShiftY;
				const int32 sectionEndPositionY = highwayScrollContext.getPositionY(y + lengthRy) + kLaserShiftY;

				// セクション開始位置でのscroll_speedの符号を取得
				const bool isStartScrollSpeedPositive = highwayScrollContext.isScrollSpeedPositiveAt(y);

				// 開始テクスチャの描画範囲を計算
				const int32 startTextureMinY = isStartScrollSpeedPositive ? sectionStartPositionY : sectionStartPositionY - kLaserStartTextureSize.y;
				const int32 startTextureMaxY = isStartScrollSpeedPositive ? sectionStartPositionY + kLaserStartTextureSize.y : sectionStartPositionY;

				// 終端が直角の場合のtailの描画範囲を計算
				const bool isEndSlam = (laserSection.v.rbegin()->second.v.v != laserSection.v.rbegin()->second.v.vf);
				int32 tailMinY = sectionEndPositionY;
				int32 tailMaxY = sectionEndPositionY;
				if (isEndSlam)
				{
					const int32 slamHeight = CalcLaserSlamHeight(y + lengthRy, highwayScrollContext);
					const int32 tailHeight = CalcLaserTailHeight(y + lengthRy, highwayScrollContext);

					tailMinY = Min(sectionEndPositionY - slamHeight - tailHeight, sectionEndPositionY - slamHeight);
					tailMaxY = Max(sectionEndPositionY - slamHeight - tailHeight, sectionEndPositionY - slamHeight);
				}

				// セクション全体の描画範囲を計算
				const int32 minY = Min(sectionStartPositionY, Min(sectionEndPositionY, Min(startTextureMinY, tailMinY)));
				const int32 maxY = Max(sectionStartPositionY, Max(sectionEndPositionY, Max(startTextureMaxY, tailMaxY)));

				// LASERセクション全体が描画範囲外の場合はスキップ
				if (maxY < 0 || minY >= kHighwayTextureSize.y)
				{
					if (!hasNegativeScrollSpeed && maxY < 0)
					{
						// scroll_speedに負の値がなく、セクション全体が上にある場合はループを抜ける
						break;
					}
					continue;
				}

				// LASERセクションの判定状況をもとに描画すべきテクスチャの行を取得
				// Offモードの場合は常に通常状態のUVを使用
				const JudgmentStatus judgmentStatus = (playOption.laserJudgmentPlayMode == JudgmentPlayMode::kOff) ? JudgmentStatus::kNormal : GetLaserSectionJudgmentStatus(laneStatus, y);
				const int32 textureRow = LaserTextureRow(judgmentStatus, gameStatus.currentTimeSec);

				// LASERセクションを描画
				DrawLaserSection(laneIdx, y, laserSection, curvedPulses, highwayScrollContext, target.additiveTexture(), m_laserNoteTexture, textureRow, m_laserNoteStartTextures[laneIdx](0, kTextureColumnMain));
				DrawLaserSection(laneIdx, y, laserSection, curvedPulses, highwayScrollContext, target.invMultiplyTexture(), m_laserNoteMaskTexture, textureRow, m_laserNoteStartTextures[laneIdx](0, kTextureColumnSub));
			}
		}
	}
}
