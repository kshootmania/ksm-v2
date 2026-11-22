#pragma once
#include <Siv3D.hpp>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

namespace MusicGame::Graphics
{
	/// @brief 動画のフレームを別スレッドで先読みするクラス
	class MovieFrameReader
	{
	private:
		struct Frame
		{
			Image image;
			int32 index = 0;
			int64 sequenceNo = 0;
		};

		static constexpr int32 kReadAheadFrameCount = 3;

		VideoReader m_videoReader;
		const bool m_loop;

		// 以下はm_mutexで保護
		std::mutex m_mutex;
		std::condition_variable m_cv;
		std::deque<Frame> m_frames; // 読み込み済みのフレーム(読み込んだ順)
		Array<Image> m_imagePool;
		int32 m_targetFrameIndex = 0;
		int32 m_nextReadFrameIndex = 0;
		int32 m_endFrameIndex;

		// 読み込んだ順の通し番号(ループで先頭に戻っても増え続ける)
		int64 m_nextSequenceNo = 0;

		// 再生位置を通し番号に換算する基準
		// 最後に取得されたフレーム、またはフレーム位置の移動先
		int32 m_baseFrameIndex = 0;
		int64 m_baseSequenceNo = 0;
		bool m_stopRequested = false;

		std::thread m_thread;

		// 以下のprivateメンバ関数はm_mutexをロックした状態で呼ぶこと

		[[nodiscard]]
		int64 targetSequenceNo() const;

		[[nodiscard]]
		bool needsSeek(int32 seekThresholdFrameCount) const;

		[[nodiscard]]
		bool needsRead() const;

		void run();

	public:
		/// @param videoReader 読み込み済みのVideoReader
		/// @param loop 終端まで読み込んだ後に先頭から読み込みを続けるか
		MovieFrameReader(const VideoReader& videoReader, bool loop);

		~MovieFrameReader();

		MovieFrameReader(const MovieFrameReader&) = delete;

		MovieFrameReader& operator=(const MovieFrameReader&) = delete;

		MovieFrameReader(MovieFrameReader&&) = delete;

		MovieFrameReader& operator=(MovieFrameReader&&) = delete;

		/// @brief 再生位置を指定し、その位置に表示するフレームが読み込み済みであれば取得
		/// @param targetFrameIndex 再生位置のフレーム番号
		/// @param pImage 取得したフレームの格納先(呼び出し前の中身は再利用のため回収される)
		/// @return 新しいフレームを取得したか
		bool fetch(int32 targetFrameIndex, Image* pImage);
	};
}
