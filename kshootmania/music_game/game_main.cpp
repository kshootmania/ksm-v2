#include "game_main.hpp"
#include "game_defines.hpp"
#include "kson/io/ksh_io.hpp"

namespace
{
	using namespace MusicGame;

	template <std::size_t N>
	int32 SumScoreValue(const std::array<Judgment::ButtonLaneJudgment, N>& laneJudgements)
	{
		int32 sum = 0;
		for (const auto& laneJudgment : laneJudgements)
		{
			sum += laneJudgment.scoreValue();
		}
		return sum;
	}

	template <std::size_t N>
	int32 SumScoreValueMax(const std::array<Judgment::ButtonLaneJudgment, N>& laneJudgements)
	{
		int32 sum = 0;
		for (const auto& laneJudgment : laneJudgements)
		{
			sum += laneJudgment.scoreValueMax();
		}
		return sum;
	}
}

void MusicGame::GameMain::updateGameStatus()
{
	const double currentTimeSec = m_bgm.posSec();
	const kson::Pulse currentPulse = kson::TimingUtils::SecToPulse(currentTimeSec, m_chartData.beat, m_timingCache);
	const double currentBPM = kson::TimingUtils::PulseTempo(currentPulse, m_chartData.beat);
	m_gameStatus.currentTimeSec = currentTimeSec;
	m_gameStatus.currentPulse = currentPulse;
	m_gameStatus.currentBPM = currentBPM;

	// BT lane judgments
	for (std::size_t i = 0U; i < kson::kNumBTLanes; ++i)
	{
		m_btLaneJudgments[i].update(m_chartData.note.btLanes[i], currentPulse, currentTimeSec, m_gameStatus.btLaneStatus[i]);
	}

	// FX lane judgments
	for (std::size_t i = 0U; i < kson::kNumFXLanes; ++i)
	{
		m_fxLaneJudgments[i].update(m_chartData.note.fxLanes[i], currentPulse, currentTimeSec, m_gameStatus.fxLaneStatus[i]);
	}

	m_gameStatus.score = static_cast<int32>(static_cast<int64>(kScoreMax) * (SumScoreValue(m_btLaneJudgments) + SumScoreValue(m_fxLaneJudgments)) / m_scoreValueMax); // TODO: add laser

	// TODO: Calculate camera values
}

MusicGame::GameMain::GameMain(const GameCreateInfo& gameCreateInfo)
	: m_chartData(kson::LoadKSHChartData(gameCreateInfo.chartFilePath.narrow()))
	, m_timingCache(kson::TimingUtils::CreateTimingCache(m_chartData.beat))
	, m_btLaneJudgments{
			Judgment::ButtonLaneJudgment(kBTButtons[0], m_chartData.note.btLanes[0], m_chartData.beat, m_timingCache),
			Judgment::ButtonLaneJudgment(kBTButtons[1], m_chartData.note.btLanes[1], m_chartData.beat, m_timingCache),
			Judgment::ButtonLaneJudgment(kBTButtons[2], m_chartData.note.btLanes[2], m_chartData.beat, m_timingCache),
			Judgment::ButtonLaneJudgment(kBTButtons[3], m_chartData.note.btLanes[3], m_chartData.beat, m_timingCache),
		}
	, m_fxLaneJudgments{
			Judgment::ButtonLaneJudgment(kFXButtons[0], m_chartData.note.fxLanes[0], m_chartData.beat, m_timingCache),
			Judgment::ButtonLaneJudgment(kFXButtons[1], m_chartData.note.fxLanes[1], m_chartData.beat, m_timingCache),
		}
	, m_scoreValueMax(SumScoreValueMax(m_btLaneJudgments) + SumScoreValueMax(m_fxLaneJudgments)) // TODO: add laser
	, m_bgm(FileSystem::ParentPath(gameCreateInfo.chartFilePath) + U"/" + Unicode::FromUTF8(m_chartData.audio.bgm.filename))
	, m_assistTick(gameCreateInfo.enableAssistTick)
	, m_audioEffectMain(m_bgm, m_chartData, m_timingCache)
	, m_musicGameGraphics(m_chartData, m_timingCache)
{
	m_bgm.seekPosSec(-TimeSecBeforeStart(false/* TODO: movie */));
	m_bgm.play();
}

void MusicGame::GameMain::update()
{
	m_bgm.update();

	// Game status and judgment
	updateGameStatus();

	// Audio effects
	std::array<Optional<bool>, kson::kNumFXLanes> longFXPressed;
	for (std::size_t i = 0U; i < kson::kNumFXLanes; ++i)
	{
		longFXPressed[i] = m_gameStatus.fxLaneStatus[i].longNotePressed;
	}
	m_audioEffectMain.update(m_bgm, m_chartData, m_timingCache, {
		.longFXPressed = longFXPressed,
	});

	// SE
	const double currentTimeSec = m_bgm.posSec();
	m_assistTick.update(m_chartData, m_timingCache, currentTimeSec);

	// Graphics
	m_musicGameGraphics.update(m_chartData, m_gameStatus);
}

void MusicGame::GameMain::draw() const
{
	m_musicGameGraphics.draw(m_chartData, m_gameStatus);
}
