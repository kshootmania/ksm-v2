#include "BGM.hpp"

namespace MusicGame::Audio
{
	LegacyAudioFPMode DetermineLegacyAudioFPMode(const kson::ChartData& chartData, const FilePath& parentPath)
	{
		const auto& legacy = chartData.audio.bgm.legacy;

		const FilePath pathF = FileSystem::PathAppend(parentPath, Unicode::FromUTF8(legacy.filenameF));
		const FilePath pathP = FileSystem::PathAppend(parentPath, Unicode::FromUTF8(legacy.filenameP));
		const FilePath pathFP = FileSystem::PathAppend(parentPath, Unicode::FromUTF8(legacy.filenameFP));

		const bool hasF = !legacy.filenameF.empty() && FileSystem::Exists(pathF);
		const bool hasP = !legacy.filenameP.empty() && FileSystem::Exists(pathP);
		const bool hasFP = !legacy.filenameFP.empty() && FileSystem::Exists(pathFP);

		if (hasF && hasP && hasFP)
		{
			return LegacyAudioFPMode::kFP;
		}
		else if (hasF)
		{
			return LegacyAudioFPMode::kF;
		}
		else
		{
			return LegacyAudioFPMode::kNone;
		}
	}

	namespace
	{
		constexpr Duration kBlendTime = 5s;
		constexpr Duration kManualUpdateInterval = 0.005s;

		void EmplaceAudioEffectToBus(
			ksmaudio::AudioEffect::AudioEffectBus* pAudioEffectBus,
			const std::string& name,
			const kson::AudioEffectDef& def,
			const std::unordered_map<std::string, std::map<float, std::string>>& paramChanges,
			const std::set<float>& updateTriggerTiming)
		{
			switch (def.type)
			{
			case kson::AudioEffectType::Retrigger:
				pAudioEffectBus->emplaceAudioEffect<ksmaudio::Retrigger>(name, def.v, paramChanges, updateTriggerTiming);
				break;

			case kson::AudioEffectType::Gate:
				pAudioEffectBus->emplaceAudioEffect<ksmaudio::Gate>(name, def.v, paramChanges, updateTriggerTiming);
				break;

			case kson::AudioEffectType::Flanger:
				pAudioEffectBus->emplaceAudioEffect<ksmaudio::Flanger>(name, def.v, paramChanges, updateTriggerTiming);
				break;

			case kson::AudioEffectType::Bitcrusher:
				pAudioEffectBus->emplaceAudioEffect<ksmaudio::Bitcrusher>(name, def.v, paramChanges, updateTriggerTiming);
				break;

			case kson::AudioEffectType::Phaser:
				pAudioEffectBus->emplaceAudioEffect<ksmaudio::Phaser>(name, def.v, paramChanges, updateTriggerTiming);
				break;

			case kson::AudioEffectType::PitchShift:
				pAudioEffectBus->emplaceAudioEffect<ksmaudio::PitchShift>(name, def.v, paramChanges, updateTriggerTiming);
				break;

			case kson::AudioEffectType::Wobble:
				pAudioEffectBus->emplaceAudioEffect<ksmaudio::Wobble>(name, def.v, paramChanges, updateTriggerTiming);
				break;

			case kson::AudioEffectType::Tapestop:
				pAudioEffectBus->emplaceAudioEffect<ksmaudio::Tapestop>(name, def.v, paramChanges, updateTriggerTiming);
				break;

			case kson::AudioEffectType::Echo:
				pAudioEffectBus->emplaceAudioEffect<ksmaudio::Echo>(name, def.v, paramChanges, updateTriggerTiming);
				break;

			case kson::AudioEffectType::Sidechain:
				pAudioEffectBus->emplaceAudioEffect<ksmaudio::Sidechain>(name, def.v, paramChanges, updateTriggerTiming);
				break;

			case kson::AudioEffectType::PeakingFilter:
				pAudioEffectBus->emplaceAudioEffect<ksmaudio::PeakingFilter>(name, def.v, paramChanges, updateTriggerTiming);
				break;

			case kson::AudioEffectType::HighPassFilter:
				pAudioEffectBus->emplaceAudioEffect<ksmaudio::HighPassFilter>(name, def.v, paramChanges, updateTriggerTiming);
				break;

			case kson::AudioEffectType::LowPassFilter:
				pAudioEffectBus->emplaceAudioEffect<ksmaudio::LowPassFilter>(name, def.v, paramChanges, updateTriggerTiming);
				break;

			default:
				assert(false && "Unknown audio effect type");
				break;
			}
		}
	}

	void LegacyAudioFPStream::load(const kson::ChartData& chartData, const FilePath& parentPath, double volume, [[maybe_unused]] SecondsF offset, double playbackSpeed)
	{
		if (mode == LegacyAudioFPMode::kNone)
		{
			return;
		}

		const auto& legacy = chartData.audio.bgm.legacy;

		const FilePath pathF = FileSystem::PathAppend(parentPath, Unicode::FromUTF8(legacy.filenameF));
		const FilePath pathP = FileSystem::PathAppend(parentPath, Unicode::FromUTF8(legacy.filenameP));
		const FilePath pathFP = FileSystem::PathAppend(parentPath, Unicode::FromUTF8(legacy.filenameFP));

		if (mode == LegacyAudioFPMode::kFP)
		{
			streamF = std::make_unique<ksmaudio::StreamWithEffects>(pathF.toUTF8(), volume, true, true, playbackSpeed);
			streamP = std::make_unique<ksmaudio::StreamWithEffects>(pathP.toUTF8(), volume, true, true, playbackSpeed);
			streamFP = std::make_unique<ksmaudio::StreamWithEffects>(pathFP.toUTF8(), volume, true, true, playbackSpeed);

			streamF->setMuted(true);
			streamP->setMuted(true);
			streamFP->setMuted(true);
		}
		else if (mode == LegacyAudioFPMode::kF)
		{
			streamF = std::make_unique<ksmaudio::StreamWithEffects>(pathF.toUTF8(), volume, true, true, playbackSpeed);
			pAudioEffectBusLaserForF = streamF->emplaceAudioEffectBusLaser();

			streamF->setMuted(true);
		}
	}

	void LegacyAudioFPStream::update([[maybe_unused]] ksmaudio::StreamWithEffects& mainStream)
	{
		if (mode == LegacyAudioFPMode::kNone)
		{
			return;
		}

		if (streamF)
		{
			streamF->updateManually();
		}
		if (streamP)
		{
			streamP->updateManually();
		}
		if (streamFP)
		{
			streamFP->updateManually();
		}
	}

	void LegacyAudioFPStream::updateMute(ksmaudio::StreamWithEffects& mainStream, bool fxActive, bool laserActive)
	{
		if (mode == LegacyAudioFPMode::kNone)
		{
			return;
		}

		if (mode == LegacyAudioFPMode::kFP)
		{
			const bool useMain = !fxActive && !laserActive;
			const bool useF = fxActive && !laserActive;
			const bool useP = !fxActive && laserActive;
			const bool useFP = fxActive && laserActive;

			mainStream.setMuted(!useMain);
			streamF->setMuted(!useF);
			streamP->setMuted(!useP);
			streamFP->setMuted(!useFP);
		}
		else if (mode == LegacyAudioFPMode::kF)
		{
			const bool useMain = !fxActive;
			const bool useF = fxActive;

			mainStream.setMuted(!useMain);
			streamF->setMuted(!useF);
		}
	}

	void LegacyAudioFPStream::play()
	{
		if (streamF)
		{
			streamF->play();
		}
		if (streamP)
		{
			streamP->play();
		}
		if (streamFP)
		{
			streamFP->play();
		}
	}

	void LegacyAudioFPStream::pause()
	{
		if (streamF)
		{
			streamF->pause();
		}
		if (streamP)
		{
			streamP->pause();
		}
		if (streamFP)
		{
			streamFP->pause();
		}
	}

	void LegacyAudioFPStream::stop()
	{
		if (streamF)
		{
			streamF->stop();
		}
		if (streamP)
		{
			streamP->stop();
		}
		if (streamFP)
		{
			streamFP->stop();
		}
	}

	void LegacyAudioFPStream::seekPosSec(SecondsF posSec)
	{
		if (streamF)
		{
			streamF->seekPosSec(posSec);
		}
		if (streamP)
		{
			streamP->seekPosSec(posSec);
		}
		if (streamFP)
		{
			streamFP->seekPosSec(posSec);
		}
	}

	void LegacyAudioFPStream::updateManually()
	{
		if (streamF)
		{
			streamF->updateManually();
		}
		if (streamP)
		{
			streamP->updateManually();
		}
		if (streamFP)
		{
			streamFP->updateManually();
		}
	}

	void LegacyAudioFPStream::emplaceAudioEffectLaser(
		const std::string& name,
		const kson::AudioEffectDef& def,
		const std::unordered_map<std::string, std::map<float, std::string>>& paramChanges,
		const std::set<float>& updateTriggerTiming)
	{
		if (mode != LegacyAudioFPMode::kF || !pAudioEffectBusLaserForF)
		{
			return;
		}

		EmplaceAudioEffectToBus(pAudioEffectBusLaserForF, name, def, paramChanges, updateTriggerTiming);
	}

	void LegacyAudioFPStream::updateAudioEffectLaser(
		bool bypass,
		const ksmaudio::AudioEffect::Status& status,
		const std::optional<std::size_t>& activeAudioEffectIdx,
		const ksmaudio::AudioEffect::AudioEffectBus& mainAudioEffectBusLaser)
	{
		if (mode != LegacyAudioFPMode::kF || !pAudioEffectBusLaserForF)
		{
			return;
		}

		if (!streamF || streamF->muted())
		{
			return;
		}

		pAudioEffectBusLaserForF->setBypass(bypass);

		const std::optional<std::size_t> convertedIdx = activeAudioEffectIdx.has_value()
			? mainAudioEffectBusLaser.convertIdxToOtherBus(activeAudioEffectIdx.value(), *pAudioEffectBusLaserForF)
			: std::nullopt;

		pAudioEffectBusLaserForF->updateByLaser(status, convertedIdx);
	}


	void BGM::emplaceAudioEffectImpl(bool isFX, const std::string& name, const kson::AudioEffectDef& def, const std::unordered_map<std::string, std::map<float, std::string>>& paramChanges, const std::set<float>& updateTriggerTiming)
	{
		if (m_stream.numChannels() == 0)
		{
			// ロード失敗時は音声エフェクトを追加しない
			return;
		}

		const auto pAudioEffectBus = isFX ? m_pAudioEffectBusFX : m_pAudioEffectBusLaser;
		EmplaceAudioEffectToBus(pAudioEffectBus, name, def, paramChanges, updateTriggerTiming);
	}

	BGM::BGM(FilePathView filePath, double volume, SecondsF offset, LegacyAudioFPMode legacyMode, const kson::ChartData& chartData, const FilePath& parentPath, double playbackSpeed)
		: m_stream(filePath.toUTF8(), volume, true, true, playbackSpeed)
		, m_playbackSpeed(playbackSpeed)
		, m_duration(m_stream.duration())
		, m_offset(offset)
		, m_pAudioEffectBusFX(m_stream.emplaceAudioEffectBusFX())
		, m_pAudioEffectBusLaser(m_stream.emplaceAudioEffectBusLaser())
		, m_stopwatch(StartImmediately::No)
		, m_manualUpdateStopwatch(StartImmediately::Yes)
		, m_legacyAudioFPStream{ .mode = legacyMode }
	{
		m_legacyAudioFPStream.load(chartData, parentPath, volume, offset, playbackSpeed);
	}

	void BGM::update()
	{
		if (m_isPaused)
		{
			return;
		}

		if (m_isStreamStarted)
		{
			if (m_manualUpdateStopwatch.elapsed() >= kManualUpdateInterval)
			{
				m_stream.updateManually();
				for (auto& switchAudio : m_switchAudioStreamsFX)
				{
					switchAudio->stream.updateManually();
				}
				for (auto& switchAudio : m_switchAudioStreamsLaser)
				{
					switchAudio->stream.updateManually();
				}
				m_legacyAudioFPStream.updateManually();
				m_manualUpdateStopwatch.restart();
			}
			m_timeSec = m_stream.posSec() - m_offset;

			if (m_timeSec + m_offset < m_duration - kBlendTime)
			{
				// ストップウォッチの時間を同期
				m_stopwatch.set(SecondsF{ m_timeSec });
			}
		}
		else
		{
			m_timeSec = m_stopwatch.elapsed();

			if (m_timeSec + m_offset >= 0s)
			{
				m_stream.seekPosSec(m_timeSec + m_offset);
				m_stream.play();
				for (auto& switchAudio : m_switchAudioStreamsFX)
				{
					switchAudio->stream.seekPosSec(m_timeSec + m_offset);
					switchAudio->stream.play();
				}
				for (auto& switchAudio : m_switchAudioStreamsLaser)
				{
					switchAudio->stream.seekPosSec(m_timeSec + m_offset);
					switchAudio->stream.play();
				}
				m_legacyAudioFPStream.seekPosSec(m_timeSec + m_offset);
				m_legacyAudioFPStream.play();
				m_isStreamStarted = true;
			}
		}
	}

	void BGM::updateAudioEffectFX(bool bypass, const ksmaudio::AudioEffect::Status& status, const ksmaudio::AudioEffect::ActiveAudioEffectDict& activeAudioEffects)
	{
		m_pAudioEffectBusFX->setBypass(bypass);
		m_pAudioEffectBusFX->updateByFX(
			status,
			activeAudioEffects);
	}

	void BGM::updateAudioEffectLaser(bool bypass, const ksmaudio::AudioEffect::Status& status, const std::optional<std::size_t>& activeAudioEffectIdx)
	{
		// メイン音源へのLASERエフェクト適用
		m_pAudioEffectBusLaser->setBypass(bypass);
		m_pAudioEffectBusLaser->updateByLaser(
			status,
			activeAudioEffectIdx);

		// FX用SwitchAudio音源へのLASERエフェクト適用
		for (auto& switchAudio : m_switchAudioStreamsFX)
		{
			switchAudio->pAudioEffectBusLaser->setBypass(bypass);

			// SwitchAudioストリームには標準LASERエフェクト(peak,hpf,lpf,bitc)のみ登録されているので、インデックスを変換
			// ユーザー定義エフェクト(isBuiltin=false)の場合はnulloptとなる
			const std::optional<std::size_t> convertedIdx = activeAudioEffectIdx.has_value()
				? m_pAudioEffectBusLaser->convertIdxToOtherBus(activeAudioEffectIdx.value(), *switchAudio->pAudioEffectBusLaser)
				: std::nullopt;

			switchAudio->pAudioEffectBusLaser->updateByLaser(
				status,
				convertedIdx);
		}

		// レガシーf音源へのLASERエフェクト適用
		m_legacyAudioFPStream.updateAudioEffectLaser(bypass, status, activeAudioEffectIdx, *m_pAudioEffectBusLaser);
	}

	void BGM::play()
	{
		m_stopwatch.start();
		m_isStreamStarted = false;
		m_isPaused = false;
	}

	void BGM::pause()
	{
		if (m_isStreamStarted)
		{
			m_stream.pause();
			for (auto& switchAudio : m_switchAudioStreamsFX)
			{
				switchAudio->stream.pause();
			}
			for (auto& switchAudio : m_switchAudioStreamsLaser)
			{
				switchAudio->stream.pause();
			}
			m_legacyAudioFPStream.pause();
		}
		m_stopwatch.pause();
		m_isPaused = true;
	}

	void BGM::seekPosSec(SecondsF posSec)
	{
		if (posSec < 0s)
		{
			m_stream.stop();
			for (auto& switchAudio : m_switchAudioStreamsFX)
			{
				switchAudio->stream.stop();
			}
			for (auto& switchAudio : m_switchAudioStreamsLaser)
			{
				switchAudio->stream.stop();
			}
			m_legacyAudioFPStream.stop();
		}
		else
		{
			m_stream.seekPosSec(posSec);
			for (auto& switchAudio : m_switchAudioStreamsFX)
			{
				switchAudio->stream.seekPosSec(posSec);
			}
			for (auto& switchAudio : m_switchAudioStreamsLaser)
			{
				switchAudio->stream.seekPosSec(posSec);
			}
			m_legacyAudioFPStream.seekPosSec(posSec);
		}
		m_timeSec = posSec;
		m_stopwatch.set(posSec);
	}

	SecondsF BGM::posSec() const
	{
		// 開始・終了地点でノーツの動きが一瞬止まるのを防ぐため、最初と最後は再生位置に対してストップウォッチの時間を混ぜる
		// TODO: うまく効いていないようなので見直す
		if (m_isStreamStarted)
		{
			const SecondsF timeSecWithOffset = m_timeSec + m_offset;
			if (0s <= timeSecWithOffset && timeSecWithOffset < kBlendTime)
			{
				const double lerpRate = timeSecWithOffset / kBlendTime;
				return Math::Lerp(m_stopwatch.elapsed(), m_timeSec, lerpRate);
			}
			else if (m_duration - kBlendTime <= timeSecWithOffset && timeSecWithOffset < m_duration)
			{
				const double lerpRate = (timeSecWithOffset - (m_duration - kBlendTime)) / kBlendTime;
				return Math::Lerp(m_timeSec, m_stopwatch.elapsed(), lerpRate);
			}
			else if (m_duration - m_offset <= m_stopwatch.elapsed())
			{
				return m_stopwatch.elapsed();
			}
		}

		return m_timeSec;
	}

	Duration BGM::duration() const
	{
		return m_duration;
	}

	Duration BGM::latency() const
	{
		return m_stream.latency();
	}

	void BGM::emplaceAudioEffectFX(const std::string& name, const kson::AudioEffectDef& def, const std::unordered_map<std::string, std::map<float, std::string>>& paramChanges, const std::set<float>& updateTriggerTiming)
	{
		emplaceAudioEffectImpl(true, name, def, paramChanges, updateTriggerTiming);
	}

	void BGM::emplaceAudioEffectLaser(const std::string& name, const kson::AudioEffectDef& def, const std::unordered_map<std::string, std::map<float, std::string>>& paramChanges, const std::set<float>& updateTriggerTiming)
	{
		emplaceAudioEffectImpl(false, name, def, paramChanges, updateTriggerTiming);
	}

	const ksmaudio::AudioEffect::AudioEffectBus& BGM::audioEffectBusFX() const
	{
		return *m_pAudioEffectBusFX;
	}

	const ksmaudio::AudioEffect::AudioEffectBus& BGM::audioEffectBusLaser() const
	{
		return *m_pAudioEffectBusLaser;
	}

	void BGM::setFadeOut(Duration duration)
	{
		m_stream.setFadeOut(duration);
	}

	void BGM::emplaceSwitchAudioStream(
		bool isFX,
		const std::string& effectName,
		const std::string& filename,
		const FilePath& parentPath,
		double volume)
	{
		const FilePath fullPath = FileSystem::PathAppend(parentPath, Unicode::FromUTF8(filename));
		if (!FileSystem::Exists(fullPath))
		{
			// 音声ファイルが見つからない場合はスキップ
			Logger << U"[ksm warning] SwitchAudio audio file not found: '{}'"_fmt(fullPath);
			return;
		}

		// ストリームを追加
		auto& targetStreams = isFX ? m_switchAudioStreamsFX : m_switchAudioStreamsLaser;
		targetStreams.emplace_back(std::make_unique<SwitchAudioStream>(
			effectName,
			fullPath.toUTF8(),
			volume,
			true,  // コンプレッサー有効
			true,  // プリロード
			m_playbackSpeed));

		// 初期状態はミュートにする
		targetStreams.back()->stream.setMuted(true);
	}

	void BGM::updateSwitchAudio(Optional<std::size_t> switchAudioIdxFX, Optional<std::size_t> switchAudioIdxLaser)
	{
		if (m_activeSwitchAudioIdxFX == switchAudioIdxFX && m_activeSwitchAudioIdxLaser == switchAudioIdxLaser)
		{
			return;  // 変更なし
		}

		// SwitchAudio切り替えの優先順位は「FXのSwitchAudio＞LASERのSwitchAudio＞メイン」とする(v1と同じ)
		// 同時に鳴るのは必ず1つだけ
		const bool fxActive = switchAudioIdxFX.has_value();
		const bool laserActive = !fxActive && switchAudioIdxLaser.has_value();
		const bool mainActive = !fxActive && !laserActive;

		// ミュート状態に反映
		m_stream.setMuted(!mainActive);
		for (std::size_t i = 0; i < m_switchAudioStreamsFX.size(); ++i)
		{
			m_switchAudioStreamsFX[i]->stream.setMuted(!fxActive || i != switchAudioIdxFX.value());
		}
		for (std::size_t i = 0; i < m_switchAudioStreamsLaser.size(); ++i)
		{
			m_switchAudioStreamsLaser[i]->stream.setMuted(!laserActive || i != switchAudioIdxLaser.value());
		}

		m_activeSwitchAudioIdxFX = switchAudioIdxFX;
		m_activeSwitchAudioIdxLaser = switchAudioIdxLaser;
	}

	Optional<std::size_t> BGM::switchAudioIdxByNameFX(const std::string& name) const
	{
		// Note: SwitchAudioの数は少ないため、線形探索で問題ない
		for (std::size_t i = 0; i < m_switchAudioStreamsFX.size(); ++i)
		{
			if (m_switchAudioStreamsFX[i]->name == name)
			{
				return i;
			}
		}
		return none;
	}

	Optional<std::size_t> BGM::switchAudioIdxByNameLaser(const std::string& name) const
	{
		// Note: SwitchAudioの数は少ないため、線形探索で問題ない
		for (std::size_t i = 0; i < m_switchAudioStreamsLaser.size(); ++i)
		{
			if (m_switchAudioStreamsLaser[i]->name == name)
			{
				return i;
			}
		}
		return none;
	}

	void BGM::emplaceSwitchAudioLaserEffect(
		const std::string& name,
		const kson::AudioEffectDef& def,
		const std::unordered_map<std::string, std::map<float, std::string>>& paramChanges,
		const std::set<float>& updateTriggerTiming)
	{
		// FX用のSwitchAudioストリームのみにLASERエフェクトを登録
		// (標準のLASERエフェクトのみが登録される)
		for (auto& switchAudio : m_switchAudioStreamsFX)
		{
			EmplaceAudioEffectToBus(switchAudio->pAudioEffectBusLaser, name, def, paramChanges, updateTriggerTiming);
		}
	}

	void BGM::updateLegacyAudioMute(bool fxActive, bool laserActive)
	{
		m_legacyAudioFPStream.updateMute(m_stream, fxActive, laserActive);
	}

	LegacyAudioFPStream& BGM::legacyAudioFPStream()
	{
		return m_legacyAudioFPStream;
	}

	const LegacyAudioFPStream& BGM::legacyAudioFPStream() const
	{
		return m_legacyAudioFPStream;
	}

	LegacyAudioFPMode BGM::legacyAudioFPMode() const
	{
		return m_legacyAudioFPStream.mode;
	}

	double BGM::playbackSpeed() const
	{
		return m_playbackSpeed;
	}
}
