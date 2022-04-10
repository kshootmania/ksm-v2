#include "play_scene.hpp"

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
	, m_stopwatch(StartImmediately::Yes)
{
	Print << U"Received: {}"_fmt(getData().playSceneArgs.chartFilePath);
}

void PlayScene::update()
{
	const double currentTimeSec = m_stopwatch.sF(); // TODO: use music play time position

	m_graphicsUpdateInfo.currentTimeSec = currentTimeSec;

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
