#include "game_main.hpp"
#include "game_defines.hpp"
#include "audio/audio_effect_utils.hpp"
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

void MusicGame::GameMain::registerAudioEffects()
{
	const std::int64_t maxMeasureIdx = kson::TimingUtils::MsToMeasureIdx(MathUtils::SecToMs(m_bgm.durationSec()), m_chartData.beat, m_timingCache);
	const std::int64_t measureCount = maxMeasureIdx + 1;
	const std::int64_t totalMeasures = measureCount + 1;

	// FX
	for (const auto& [name, def] : m_chartData.audio.audioEffects.fx.def)
	{
		const auto& paramChangeDict = m_chartData.audio.audioEffects.fx.paramChange;
		const std::set<float> updateTriggerTiming =
			paramChangeDict.contains(name)
				? Audio::AudioEffectUtils::PrecalculateUpdateTriggerTiming(def, paramChangeDict.at(name), totalMeasures, m_chartData, m_timingCache)
				: Audio::AudioEffectUtils::PrecalculateUpdateTriggerTiming(def, totalMeasures, m_chartData, m_timingCache);

		m_bgm.emplaceAudioEffect(true, name, def, updateTriggerTiming);
	}

	// Laser
	for (const auto& [name, def] : m_chartData.audio.audioEffects.laser.def)
	{
		const auto& paramChangeDict = m_chartData.audio.audioEffects.laser.paramChange;
		const std::set<float> updateTriggerTiming =
			paramChangeDict.contains(name)
			? Audio::AudioEffectUtils::PrecalculateUpdateTriggerTiming(def, paramChangeDict.at(name), totalMeasures, m_chartData, m_timingCache)
			: Audio::AudioEffectUtils::PrecalculateUpdateTriggerTiming(def, totalMeasures, m_chartData, m_timingCache);

		m_bgm.emplaceAudioEffect(false, name, def, updateTriggerTiming);
	}

	const kson::AudioEffectDef def_ = kson::AudioEffectDef{
		.type = kson::AudioEffectType::Retrigger,
		.v = {
			{ "wave_length", "1/8" },
			{ "mix", "0%>100%" },
		},
	};
	const auto updateTriggerTiming_ = Audio::AudioEffectUtils::PrecalculateUpdateTriggerTiming(
		def_, totalMeasures, m_chartData, m_timingCache);
	m_bgm.emplaceAudioEffect(true, "retr", def_, updateTriggerTiming_);
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
	, m_audioEffectUpdateInfo{ .pChartData = &m_chartData }
	, m_graphicsUpdateInfo{ .pChartData = &m_chartData }
	, m_musicGameGraphics(m_chartData, m_timingCache)
{
	registerAudioEffects();

	m_bgm.seekPosSec(-TimeSecBeforeStart(false/* TODO: movie */));
	m_bgm.play();
}

void MusicGame::GameMain::update()
{
	m_bgm.update();

	const double currentTimeSec = m_bgm.posSec();

	// TODO: Make separate functions for the code below

	// SE
	{
		m_assistTick.update(m_chartData, m_timingCache, currentTimeSec);
	}

	// Audio effects
	{
		constexpr double kLatencySec = 0.2; // TODO: determined by buffer size?
		const kson::Pulse currentPulse = kson::TimingUtils::MsToPulse(MathUtils::SecToMs(currentTimeSec + kLatencySec), m_chartData.beat, m_timingCache);
		const double currentBPM = kson::TimingUtils::PulseTempo(currentPulse, m_chartData.beat);
		m_bgm.updateAudioEffectFX({
			.v = 0.0f,
			.bpm = static_cast<float>(currentBPM),
			.sec = static_cast<float>(currentTimeSec + kLatencySec),//MathUtils::MsToSec<float>(ksmaudio::kBufferSizeMs),
		}, { "retr"/*TODO*/ });
		// TODO: laser
	}

	// Graphics and judgment
	{
		const kson::Pulse currentPulse = kson::TimingUtils::MsToPulse(MathUtils::SecToMs(currentTimeSec), m_chartData.beat, m_timingCache);
		const double currentBPM = kson::TimingUtils::PulseTempo(currentPulse, m_chartData.beat);
		m_graphicsUpdateInfo.currentTimeSec = currentTimeSec;
		m_graphicsUpdateInfo.currentPulse = currentPulse;
		m_graphicsUpdateInfo.currentBPM = currentBPM;
		for (int i = 0; i < kson::kNumBTLanes; ++i)
		{
			m_btLaneJudgments[i].update(m_chartData.note.btLanes[i], currentPulse, currentTimeSec, m_graphicsUpdateInfo.btLaneState[i]);
		}
		for (int i = 0; i < kson::kNumFXLanes; ++i)
		{
			m_fxLaneJudgments[i].update(m_chartData.note.fxLanes[i], currentPulse, currentTimeSec, m_graphicsUpdateInfo.fxLaneState[i]);
		}
		m_graphicsUpdateInfo.score = static_cast<int32>(static_cast<int64>(kScoreMax) * (SumScoreValue(m_btLaneJudgments) + SumScoreValue(m_fxLaneJudgments)) / m_scoreValueMax); // TODO: add laser
	}

	m_musicGameGraphics.update(m_graphicsUpdateInfo);
}

void MusicGame::GameMain::draw() const
{
	// TODO: Calculate camera state
	m_musicGameGraphics.draw();
}
