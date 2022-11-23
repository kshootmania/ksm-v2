#include "laser_note_graphics.hpp"
#include "note_graphics_utils.hpp"
#include "music_game/graphics/graphics_defines.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr StringView kLaserNoteTextureFilename = U"laser.gif";
		constexpr StringView kLaserNoteMaskTextureFilename = U"laser_mask.gif";
		constexpr StringView kLaserNoteLeftStartTextureFilename = U"laserl_0.gif";
		constexpr StringView kLaserNoteRightStartTextureFilename = U"laserr_0.gif";

		constexpr Size kLaserTextureSize = { 48, 48 };
		constexpr double kLaserLineWidth = static_cast<double>(kLaserTextureSize.x);
		constexpr Size kLaserStartTextureSize = { 44, 200 };
		constexpr double kLaserShiftY = -10.0;
		constexpr double kLaserTailHeight = 80.0;

		constexpr double kLaserCriticalBlinkIntervalSec = 0.12;

		enum : int32
		{
			kLaserNoteStartTextureColumnAdditive = 0,
			kLaserNoteStartTextureColumnInvMultiply,
		};

		enum class JudgmentStatus
		{
			kNormal,
			kCritical,
			kError,
		};

		Quad LaserLineQuad(const Vec2& positionStart, const Vec2& positionEnd)
		{
			return {
				positionStart + Vec2{ kLaserLineWidth / 2, 0.0 },
				positionStart + Vec2{ -kLaserLineWidth / 2, 0.0 },
				positionEnd + Vec2{ -kLaserLineWidth / 2, 0.0 },
				positionEnd + Vec2{ kLaserLineWidth / 2, 0.0 }
			};
		}

		void DrawLaserLine(int32 laneIdx, kson::RelPulse relPulse1, const kson::GraphValue& point1, kson::RelPulse relPulse2, const kson::GraphValue& point2, const Texture& laserNoteTexture, int32 laserNoteTextureRow)
		{
			const Vec2 positionStart = {
				point1.vf * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
				static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse1) * 480 / kson::kResolution + kLaserShiftY - ((point1.v != point1.vf) ? kLaserTextureSize.y : 0.0)
			};

			const Vec2 positionEnd = {
				point2.v * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
				static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse2) * 480 / kson::kResolution + kLaserShiftY
			};

			const Quad quad = LaserLineQuad(positionStart, positionEnd);
			quad(laserNoteTexture(kLaserTextureSize.x * laneIdx, kLaserTextureSize.y * laserNoteTextureRow + kLaserTextureSize.y - 1 + kOnePixelTextureSourceOffset, kLaserTextureSize.x, kOnePixelTextureSourceSize)).draw();
		}

		Quad LaserSlamLineQuad(const Vec2& positionStart, const Vec2& positionEnd)
		{
			if (Abs(positionEnd.x - positionStart.x) <= kLaserLineWidth)
			{
				// 左右が近すぎて直角レーザーの横線が描画できない場合はゼロを返す
				return { Vec2::Zero(), Vec2::Zero(), Vec2::Zero(), Vec2::Zero() };
			}

			const int diffXSign = Sign(positionEnd.x - positionStart.x);
			return {
				positionStart + Vec2{ diffXSign * kLaserLineWidth / 2, -kLaserLineWidth },
				positionEnd + Vec2{ -diffXSign * kLaserLineWidth / 2, -kLaserLineWidth },
				positionEnd + Vec2{ -diffXSign * kLaserLineWidth / 2, 0.0 },
				positionStart + Vec2{ diffXSign * kLaserLineWidth / 2, 0.0 }
			};
		}

		void DrawLaserSlam(int32 laneIdx, kson::RelPulse relPulse, const kson::GraphValue& point, const Texture& laserNoteTexture, int32 laserNoteTextureRow)
		{
			const Vec2 positionStart = {
				point.v * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
				static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse) * 480 / kson::kResolution + kLaserShiftY
			};

			const Vec2 positionEnd = {
				point.vf * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
				static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse) * 480 / kson::kResolution + kLaserShiftY
			};

			// 直角レーザーの角のテクスチャを描画
			const bool isLeftToRight = (point.v < point.vf);
			laserNoteTexture(kLaserTextureSize.x * laneIdx, 0, kLaserTextureSize).mirrored(isLeftToRight).drawAt(positionStart + Vec2{ 0.0, -kLaserLineWidth / 2 });
			laserNoteTexture(kLaserTextureSize.x * laneIdx, 0, kLaserTextureSize).mirrored(!isLeftToRight).flipped().drawAt(positionEnd + Vec2{ 0.0, -kLaserLineWidth / 2 });

			// 直角レーザーの横線を描画
			const Quad quad = LaserSlamLineQuad(positionStart, positionEnd);
			quad(laserNoteTexture(kLaserTextureSize.x * laneIdx + kOnePixelTextureSourceOffset, kLaserTextureSize.y * laserNoteTextureRow, kOnePixelTextureSourceSize, kLaserTextureSize.y)).draw();
		}

		void DrawLaserSlamTail(int32 laneIdx, kson::RelPulse relPulse, const kson::GraphValue& point, const Texture& laserNoteTexture, int32 laserNoteTextureRow)
		{
			const Vec2 positionStart = {
				point.vf * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
				static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse) * 480 / kson::kResolution + kLaserShiftY
			};
			const Quad quad = LaserLineQuad(positionStart + Vec2{ 0.0, -kLaserTextureSize.y }, positionStart + Vec2{ 0.0, -kLaserTextureSize.y - kLaserTailHeight });
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

		void DrawLaserSection(int32 laneIdx, kson::RelPulse relPulse, const kson::LaserSection& laserSection, const RenderTexture& target, const Texture& laserNoteTexture, int32 laserNoteTextureRow, const TextureRegion& laserStartTexture)
		{
			const ScopedRenderTarget2D renderTarget(target);

			for (auto itr = laserSection.v.begin(); itr != laserSection.v.end(); ++itr)
			{
				const auto& [ry, point] = *itr;

				// レーザー開始テクスチャを描画
				if (itr == laserSection.v.begin())
				{
					const Vec2 positionStart = {
						point.v * (kHighwayTextureSize.x - kLaserLineWidth) + kLaserLineWidth / 2,
						static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse + ry) * 480 / kson::kResolution + kLaserShiftY
					};
					laserStartTexture.draw(Arg::topCenter = positionStart);
				}

				const double positionStartY = static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse + ry) * 480 / kson::kResolution + kLaserShiftY;
				if (positionStartY < 0)
				{
					// レーザーの線を構成する2つの点(始点・終点)のうち始点が描画範囲より上にある場合は描画しない
					// それ以降のレーザーも上にあるため描画対象外となるので、ここでreturnする
					return;
				}

				// 直角レーザーを描画
				if (point.v != point.vf)
				{
					DrawLaserSlam(laneIdx, relPulse + ry, point, laserNoteTexture, laserNoteTextureRow);
				}

				// レーザー終端の点の場合は線を描画しない
				const auto nextItr = std::next(itr);
				if (nextItr == laserSection.v.end())
				{
					// 終端が直角の場合は終端を伸ばす
					if (point.v != point.vf)
					{
						DrawLaserSlamTail(laneIdx, relPulse + ry, point, laserNoteTexture, laserNoteTextureRow);
					}

					break;
				}

				// レーザーの2つの点をもとに線を描画
				{
					const auto& [nextRy, nextPoint] = *nextItr;
					if (relPulse + nextRy < -kson::kResolution4)
					{
						continue;
					}
					DrawLaserLine(laneIdx, relPulse + ry, point, relPulse + nextRy, nextPoint, laserNoteTexture, laserNoteTextureRow);
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
					.column = 2,
					.sourceSize = kLaserStartTextureSize,
				}),
			TiledTexture(
				kLaserNoteRightStartTextureFilename,
				{
					.column = 2,
					.sourceSize = kLaserStartTextureSize,
				}) }
	{
	}

	void LaserNoteGraphics::draw(const kson::ChartData& chartData, const GameStatus& gameStatus, const RenderTexture& additiveTarget, const RenderTexture& invMultiplyTarget) const
	{
		const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);
		const ScopedRenderStates2D renderState(BlendState::Additive);

		// LASERノーツを描画
		for (int32 laneIdx = 0; laneIdx < kson::kNumLaserLanes; ++laneIdx) // 座標計算で結局int32にする必要があるのでここではsize_t不使用
		{
			const auto& lane = chartData.note.laser[laneIdx];
			const auto& laneStatus = gameStatus.laserLaneStatus[laneIdx];

			for (auto itr = kson::ValueItrAt(lane, gameStatus.currentPulse); itr != lane.end(); ++itr)
			{
				const auto& [y, laserSection] = *itr;
				const kson::RelPulse relPulse = y - gameStatus.currentPulse;

				const double positionSectionEndY = -static_cast<double>(relPulse + laserSection.v.rbegin()->first) * 480 / kson::kResolution + kLaserShiftY - kLaserTailHeight;
				if (positionSectionEndY > 0)
				{
					// レーザーのセクション全体が描画範囲より下にある場合は描画しない
					continue;
				}

				const double positionSectionStartY = static_cast<double>(kHighwayTextureSize.y) - static_cast<double>(relPulse) * 480 / kson::kResolution + kLaserShiftY;
				if (positionSectionStartY + kLaserStartTextureSize.y < 0)
				{
					// レーザーのセクション全体が描画範囲より上にある場合は描画しない
					break;
				}

				// LASERセクションの判定状況をもとに描画すべきテクスチャの行を取得
				const JudgmentStatus judgmentStatus = GetLaserSectionJudgmentStatus(laneStatus, y);
				const int32 textureRow = LaserTextureRow(judgmentStatus, gameStatus.currentTimeSec);

				// LASERセクションを描画
				DrawLaserSection(laneIdx, relPulse, laserSection, additiveTarget, m_laserNoteTexture, textureRow, m_laserNoteStartTextures[laneIdx](0, kLaserNoteStartTextureColumnAdditive));
				DrawLaserSection(laneIdx, relPulse, laserSection, invMultiplyTarget, m_laserNoteMaskTexture, textureRow, m_laserNoteStartTextures[laneIdx](0, kLaserNoteStartTextureColumnInvMultiply));
			}
		}
	}
}
