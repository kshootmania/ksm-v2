#include "game_main.hpp"
#include "game_defines.hpp"
#include "ksh/io/ksh_io.hpp"

MusicGame::GameMain::GameMain(const GameCreateInfo& gameCreateInfo)
	: m_chartData(ksh::LoadKSHChartData(gameCreateInfo.chartFilePath.narrow()))
	, m_timingCache(ksh::TimingUtils::CreateTimingCache(m_chartData.beat))
	, m_bgm(FileSystem::ParentPath(gameCreateInfo.chartFilePath) + U"/" + Unicode::FromUTF8(m_chartData.audio.bgmInfo.filename))
	, m_assistTick(gameCreateInfo.enableAssistTick)
	, m_graphicsUpdateInfo{ .pChartData = &m_chartData }
	, m_musicGameGraphics(m_chartData, m_timingCache)
{
	m_bgm.seekTime(-TimeSecBeforeStart(false/* TODO: movie */));
	m_bgm.play();
}

void MusicGame::GameMain::update()
{
	m_bgm.update();

	const double currentTimeSec = m_bgm.posSec();
	m_assistTick.update(m_chartData, m_timingCache, currentTimeSec);

	const ksh::Pulse currentPulse = ksh::TimingUtils::MsToPulse(MathUtils::SecToMs(currentTimeSec), m_chartData.beat, m_timingCache);

	m_graphicsUpdateInfo.currentTimeSec = currentTimeSec;
	m_graphicsUpdateInfo.currentPulse = currentPulse;
	m_graphicsUpdateInfo.currentBPM = ksh::TimingUtils::PulseTempo(currentPulse, m_chartData.beat);

	for (int i = 0; i < ksh::kNumBTLanes; ++i)
	{
		if (KeyConfig::Down(kBTButtons[i]))
		{
			m_graphicsUpdateInfo.btLaneState[i].keyBeamTimeSec = currentTimeSec;
		}
	}

	for (int i = 0; i < ksh::kNumFXLanes; ++i)
	{
		if (KeyConfig::Down(kFXButtons[i]))
		{
			m_graphicsUpdateInfo.fxLaneState[i].keyBeamTimeSec = currentTimeSec;
		}
	}

	m_musicGameGraphics.update(m_graphicsUpdateInfo);
}

void MusicGame::GameMain::draw() const
{
	// TODO: Calculate camera state
	m_musicGameGraphics.draw();
}
