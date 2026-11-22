#pragma once
#include <Siv3D.hpp>
#include "MovieFrameReader.hpp"

namespace MusicGame::Graphics
{
	/// @brief 背景として画面全体に表示する動画
	class FullScreenMovie
	{
	private:
		std::unique_ptr<MovieFrameReader> m_frameReader;
		Size m_size = Size::Zero();
		double m_fps = 0.0;
		int32 m_frameCount = 0;

		// 転送中のテクスチャを描画に使わないよう、2つを交互に使用
		std::array<DynamicTexture, 2> m_frameTextures;
		size_t m_latestTextureIndex = 0;

		Image m_frameImage;
		bool m_hasUntransferredFrame = false;
		bool m_hasTexture = false;

		double m_movieOffsetSec = 0.0;
		double m_startTimeSec = 0.0;
		double m_playbackSpeed = 1.0;
		double m_fpsScale = 1.0;
		bool m_loop = false;
		bool m_isPlaying = false;
		bool m_enabled = false;

	public:
		FullScreenMovie(const FilePath& moviePath, double movieOffsetSec, double playbackSpeed, bool loop, bool enabled);

		void prepare(double globalOffsetSec);

		void update(double currentTimeSec, bool isPaused);

		[[nodiscard]]
		bool isEnabled() const;

		/// @brief 現在の再生位置に描画するフレームがあるか
		/// @remark 再生開始前・再生終了後・フレームの読み込み前はfalse
		[[nodiscard]]
		bool hasFrame() const;

		[[nodiscard]]
		const Texture& texture() const;

		[[nodiscard]]
		Size size() const;
	};
}
