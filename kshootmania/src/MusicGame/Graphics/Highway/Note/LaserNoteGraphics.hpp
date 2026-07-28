#pragma once
#include "MusicGame/GameStatus.hpp"
#include "MusicGame/PlayOption.hpp"
#include "MusicGame/Scroll/HighwayScroll.hpp"
#include "MusicGame/Graphics/Highway/HighwayRenderTexture.hpp"

namespace MusicGame::Graphics
{
	class LaserNoteGraphics
	{
	private:
		/// @brief 描画対象のLASERセクション
		struct VisibleLaserSection
		{
			/// @brief セクション開始位置のPulse値
			kson::Pulse y;

			/// @brief セクションへのポインタ
			const kson::LaserSection* pLaserSection;

			/// @brief 判定状況に応じたテクスチャの行
			int32 textureRow;
		};

		const Texture m_laserNoteTexture;
		const Texture m_laserNoteMaskTexture;
		const std::array<TiledTexture, kson::kNumLaserLanesSZ> m_laserNoteStartTextures;

		/// @brief 描画対象のセクションを溜めるバッファ(通常テクスチャとマスクテクスチャの描画で共用)
		mutable Array<VisibleLaserSection> m_visibleLaserSections;

	public:
		LaserNoteGraphics();

		void draw(const kson::ChartData& chartData, const std::array<HashSet<kson::Pulse>, kson::kNumLaserLanesSZ>& laserCurvedPulses, const PlayOption& playOption, const GameStatus& gameStatus, const Scroll::HighwayScrollContext& highwayScrollContext, const HighwayRenderTexture& target) const;
	};
}
