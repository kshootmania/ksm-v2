#include "play_scene.hpp"
#include "music_game/music_game_defines.hpp"
#include "ksh/io/ksh_io.hpp"
#include "ksh/io/kson_io.hpp"

namespace
{
	const std::array<KeyConfig::Button, ksh::kNumBTLanes> kBTButtons = {
		KeyConfig::kBT_A,
		KeyConfig::kBT_B,
		KeyConfig::kBT_C,
		KeyConfig::kBT_D,
	};

	const std::array<KeyConfig::Button, ksh::kNumFXLanes> kFXButtons = {
		KeyConfig::kFX_L,
		KeyConfig::kFX_R,
	};
}

PlayScene::PlayScene(const InitData& initData)
	: MyScene(initData)
	, m_chartData(ksh::LoadKSHChartData(getData().playSceneArgs.chartFilePath.narrow()))
	, m_timingCache(ksh::TimingUtils::CreateTimingCache(m_chartData.beat))
	, m_musicGameGraphics(m_chartData, m_timingCache)
	, m_stopwatch(StartImmediately::Yes)
	, m_graphicsUpdateInfo{ .pChartData = &m_chartData }
{
	Print << U"Title: {}"_fmt(Unicode::Widen(ksh::UnU8(m_chartData.meta.title)));
}

void PlayScene::update()
{
	const double currentTimeSec = m_stopwatch.sF() - MusicGame::TimeSecBeforeStart(false); // TODO: use music play time position

	m_graphicsUpdateInfo.currentTimeSec = currentTimeSec;
	m_graphicsUpdateInfo.currentPulse = ksh::TimingUtils::MsToPulse(MathUtils::SecToMs(currentTimeSec), m_chartData.beat, m_timingCache);

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

void PlayScene::draw() const
{
	// TODO: Calculate camera state
	m_musicGameGraphics.draw();
}
