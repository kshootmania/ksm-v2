#include "MovieFrameReader.hpp"

namespace MusicGame::Graphics
{
	int64 MovieFrameReader::targetSequenceNo() const
	{
		int32 frameCountFromBase = m_targetFrameIndex - m_baseFrameIndex;
		if (m_loop && m_endFrameIndex > 0)
		{
			frameCountFromBase = MathUtils::WrappedMod(frameCountFromBase, m_endFrameIndex);
		}
		return m_baseSequenceNo + frameCountFromBase;
	}

	bool MovieFrameReader::needsSeek(int32 seekThresholdFrameCount) const
	{
		// 離れた位置へは順に読み込まずにフレーム位置を指定して移動する
		const int64 sequenceNo = targetSequenceNo();
		return sequenceNo < m_baseSequenceNo || sequenceNo > m_nextSequenceNo + seekThresholdFrameCount;
	}

	bool MovieFrameReader::needsRead() const
	{
		return m_nextReadFrameIndex < m_endFrameIndex && m_nextSequenceNo <= targetSequenceNo() + kReadAheadFrameCount;
	}

	void MovieFrameReader::run()
	{
		const int32 seekThresholdFrameCount = Max(static_cast<int32>(m_videoReader.getFPS()), 1);

		std::unique_lock lock(m_mutex);
		while (true)
		{
			m_cv.wait(lock, [&]
			{
				return m_stopRequested || needsSeek(seekThresholdFrameCount) || needsRead();
			});
			if (m_stopRequested)
			{
				return;
			}

			if (needsSeek(seekThresholdFrameCount))
			{
				for (auto& frame : m_frames)
				{
					m_imagePool.push_back(std::move(frame.image));
				}
				m_frames.clear();
				m_baseFrameIndex = m_targetFrameIndex;
				m_baseSequenceNo = m_nextSequenceNo;
				m_nextReadFrameIndex = m_targetFrameIndex;
				m_videoReader.setCurrentFrameIndex(static_cast<size_t>(m_nextReadFrameIndex));
				continue;
			}

			// 再生位置より前のフレームは、再生位置に表示する1つを残して捨てる
			const int64 sequenceNo = targetSequenceNo();
			while (m_frames.size() >= 2 && m_frames[1].sequenceNo <= sequenceNo)
			{
				m_imagePool.push_back(std::move(m_frames.front().image));
				m_frames.pop_front();
			}

			Image image;
			if (!m_imagePool.empty())
			{
				image = std::move(m_imagePool.back());
				m_imagePool.pop_back();
			}
			const int32 frameIndex = m_nextReadFrameIndex;

			// VideoReader::readFrameは読み込みに失敗してもtrueを返し、その場合は以前に渡した画像をそのまま返す
			// 読み込まれたフレームのアルファ値は255になるため、渡す画像のアルファ値を0にしておくことで判定する
			if (!image.isEmpty())
			{
				image[0][0].a = 0;
			}

			lock.unlock();
			const bool succeeded = m_videoReader.readFrame(image) && !image.isEmpty() && image[0][0].a != 0;
			lock.lock();

			if (succeeded)
			{
				m_frames.push_back(Frame{ .image = std::move(image), .index = frameIndex, .sequenceNo = m_nextSequenceNo });
				++m_nextSequenceNo;
				m_nextReadFrameIndex = frameIndex + 1;
			}
			else
			{
				// 読み込めなかったフレーム以降は終端として扱う
				m_endFrameIndex = frameIndex;
				m_targetFrameIndex = Clamp(m_targetFrameIndex, 0, Max(m_endFrameIndex - 1, 0));
				m_nextReadFrameIndex = m_endFrameIndex;
				m_imagePool.push_back(std::move(image));
			}

			if (m_loop && m_nextReadFrameIndex >= m_endFrameIndex && m_endFrameIndex > 0)
			{
				m_nextReadFrameIndex = 0;
				m_videoReader.setCurrentFrameIndex(0);
			}
		}
	}

	MovieFrameReader::MovieFrameReader(const VideoReader& videoReader, bool loop)
		: m_videoReader(videoReader)
		, m_loop(loop)
		, m_endFrameIndex(static_cast<int32>(videoReader.getFrameCount()))
		, m_thread([this] { run(); })
	{
	}

	MovieFrameReader::~MovieFrameReader()
	{
		{
			std::lock_guard lock(m_mutex);
			m_stopRequested = true;
		}
		m_cv.notify_one();
		m_thread.join();
	}

	bool MovieFrameReader::fetch(int32 targetFrameIndex, Image* pImage)
	{
		bool fetched = false;
		{
			std::lock_guard lock(m_mutex);
			m_targetFrameIndex = Clamp(targetFrameIndex, 0, Max(m_endFrameIndex - 1, 0));

			// 再生位置以前で最も新しいフレームを探す
			const int64 sequenceNo = targetSequenceNo();
			auto it = m_frames.end();
			for (auto candidate = m_frames.begin(); candidate != m_frames.end() && candidate->sequenceNo <= sequenceNo; ++candidate)
			{
				it = candidate;
			}
			if (it != m_frames.end())
			{
				fetched = true;
				m_baseFrameIndex = it->index;
				m_baseSequenceNo = it->sequenceNo;
				std::swap(*pImage, it->image);
				if (!it->image.isEmpty())
				{
					m_imagePool.push_back(std::move(it->image));
				}
				m_frames.erase(m_frames.begin(), it + 1);
			}
		}
		m_cv.notify_one();
		return fetched;
	}
}
