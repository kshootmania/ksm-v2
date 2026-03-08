#include "AudioEffectMain.hpp"
#include "AudioEffectUtils.hpp"
#include "ksmaudio/AudioEffect/AudioEffectBus.hpp"
#include "kson/Util/GraphUtils.hpp"

namespace MusicGame::Audio
{
	namespace
	{
		constexpr double kLongFXNoteAudioEffectAutoPlaySec = 0.03;

		constexpr std::string_view kPeakingFilterAudioEffectName = "peaking_filter";
		constexpr std::string_view kDefaultLaserAudioEffectName = kPeakingFilterAudioEffectName;

		double GetEffectivePeakingFilterDelaySec(const kson::ChartData& chartData)
		{
			if (!chartData.audio.bgm.legacy.filenameP.empty())
			{
				// p音源が指定されていてp音源の音声ファイルが存在しない場合、リアルタイムエフェクトのタイミングは
				// 旧REAPER方式を再現するために遅延ゼロとする
				return 0.0;
			}

			return chartData.audio.audioEffect.laser.peakingFilterDelay / 1000.0;
		}

		struct AudioEffectDefWithBuiltinFlag
		{
			std::string name;
			kson::AudioEffectDef def;
			bool isBuiltin; // 標準エフェクトかどうか(SwitchAudioの音声に対して登録されるかに影響)
		};

		std::unordered_map<std::string, std::map<float, std::string>> ConvertParamChangesFromPulseToSec(
			const kson::Dict<kson::ByPulse<std::string>>& paramChangeDict,
			const kson::ChartData& chartData,
			const kson::TimingCache& timingCache)
		{
			std::unordered_map<std::string, std::map<float, std::string>> result;
			for (const auto& [name, paramChange] : paramChangeDict)
			{
				std::map<float, std::string> paramResult;
				for (const auto& [y, value] : paramChange)
				{
					const float sec = static_cast<float>(kson::PulseToSec(y, chartData.beat, timingCache));
					paramResult.emplace(sec, value);
				}
				result.emplace(name, std::move(paramResult));
			}
			return result;
		}

		void RegisterAudioEffects(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache, const FilePath& parentPath, double bgmVolume)
		{
			using AudioEffectUtils::PrecalculateUpdateTriggerTiming;

			const LegacyAudioFPMode legacyMode = bgm.legacyAudioFPMode();

			const std::int64_t totalMeasures =
				kson::SecToMeasureIdx(bgm.duration().count(), chartData.beat, timingCache)
				+ 1/* 最後の小節の分を足す */
				+ 1/* インデックスを要素数にするために1を足す */;

			// legacy.filterGainに0.5以外の値が存在する場合、ビルトインのフィルタエフェクトのgain・qのパラメータ変更として反映
			// (p音源が指定されているがファイルが存在しない場合、pfiltergainの値を無視して50%で適用)
			kson::Dict<kson::Dict<kson::ByPulse<std::string>>> laserParamChangeDict = chartData.audio.audioEffect.laser.paramChange;
			const auto& filterGain = chartData.audio.audioEffect.laser.legacy.filterGain;
			const bool isLaserFallback = !chartData.audio.bgm.legacy.filenameP.empty() && legacyMode != LegacyAudioFPMode::kFP;
			if (!filterGain.empty() && !isLaserFallback)
			{
				// 0.5以外の値が含まれているかチェック
				bool hasNonDefaultValue = std::any_of(
					filterGain.begin(),
					filterGain.end(),
					[](const auto& pair) { return pair.second != 0.5; });

				if (hasNonDefaultValue)
				{
					// ユーザー定義で上書きされている場合はビルトインエフェクトとみなさない
					const bool isPeakingFilterBuiltin = !chartData.audio.audioEffect.laser.defContains("peaking_filter");
					const bool isHighPassFilterBuiltin = !chartData.audio.audioEffect.laser.defContains("high_pass_filter");
					const bool isLowPassFilterBuiltin = !chartData.audio.audioEffect.laser.defContains("low_pass_filter");

					for (const auto& [pulse, filterGainValue] : filterGain)
					{
						// peaking_filterへの適用(ビルトインの場合のみ)
						if (isPeakingFilterBuiltin)
						{
							const std::int32_t gain = static_cast<std::int32_t>(std::round(filterGainValue * 100.0));
							laserParamChangeDict["peaking_filter"]["gain"][pulse] = std::to_string(gain) + "%";
						}

						// high_pass_filter/low_pass_filterへのqパラメータ適用(ビルトインの場合のみ)
						if (isHighPassFilterBuiltin)
						{
							const double qValueHPF = std::lerp(2.0, 8.0, filterGainValue);
							laserParamChangeDict["high_pass_filter"]["q"][pulse] = std::to_string(qValueHPF);
						}
						if (isLowPassFilterBuiltin)
						{
							const double qValueLPF = std::lerp(2.0, 5.2, filterGainValue);
							laserParamChangeDict["low_pass_filter"]["q"][pulse] = std::to_string(qValueLPF);
						}
					}
				}
			}

			// デフォルトのエフェクト定義を追加
			// (もし譜面側で同名のエフェクト定義がある場合は上書きせず譜面側を優先する)
			std::vector<AudioEffectDefWithBuiltinFlag> defFX;
			std::vector<AudioEffectDefWithBuiltinFlag> defLaser;
			const auto fnInsertDefaultFX = [&defFX, &chartData](const std::string& name, const kson::AudioEffectDef& v)
			{
				if (chartData.audio.audioEffect.fx.defContains(name))
				{
					return;
				}
				defFX.push_back(
					AudioEffectDefWithBuiltinFlag{
						.name = name,
						.def = v,
						.isBuiltin = true,
					});
			};
			const auto fnInsertDefaultLaser = [&defLaser, &chartData](const std::string& name, const kson::AudioEffectDef& v)
			{
				if (chartData.audio.audioEffect.laser.defContains(name))
				{
					return;
				}
				defLaser.push_back(
					AudioEffectDefWithBuiltinFlag{
						.name = name,
						.def = v,
						.isBuiltin = true,
					});
			};

			// FX
			fnInsertDefaultFX("retrigger", { .type = kson::AudioEffectType::Retrigger });
			fnInsertDefaultFX("gate", { .type = kson::AudioEffectType::Gate });
			fnInsertDefaultFX("flanger", { .type = kson::AudioEffectType::Flanger });
			fnInsertDefaultFX("pitch_shift", { .type = kson::AudioEffectType::PitchShift });
			fnInsertDefaultFX("bitcrusher", { .type = kson::AudioEffectType::Bitcrusher });
			fnInsertDefaultFX("phaser", { .type = kson::AudioEffectType::Phaser });
			fnInsertDefaultFX("wobble", { .type = kson::AudioEffectType::Wobble });
			fnInsertDefaultFX("tapestop", { .type = kson::AudioEffectType::Tapestop });
			fnInsertDefaultFX("echo", { .type = kson::AudioEffectType::Echo });
			fnInsertDefaultFX("sidechain", { .type = kson::AudioEffectType::Sidechain });
			// ユーザー定義エフェクトを追加
			for (const auto& kvp : chartData.audio.audioEffect.fx.def)
			{
				defFX.push_back(
					AudioEffectDefWithBuiltinFlag{
						.name = kvp.name,
						.def = kvp.v,
						.isBuiltin = false,
					});
			}

			// f音源またはf,p,fp音源指定時はリアルタイムエフェクトなし
			const bool skipFXEffects = legacyMode == LegacyAudioFPMode::kF || legacyMode == LegacyAudioFPMode::kFP;
			if (!skipFXEffects)
			{
				for (const auto& [name, def, isBuiltin] : defFX)
				{
					if (def.type == kson::AudioEffectType::SwitchAudio)
					{
						// SwitchAudioの場合
						if (def.v.contains("filename"))
						{
							// ファイル名をもとに音声を追加ロード
							const std::string& filename = def.v.at("filename");
							bgm.emplaceSwitchAudioStream(true, name, filename, parentPath, bgmVolume);
						}
					}
					else
					{
						// 通常のDSPエフェクトの場合
						const auto& paramChangeDict = chartData.audio.audioEffect.fx.paramChange;
						const std::set<float> updateTriggerTiming =
							paramChangeDict.contains(name)
							? PrecalculateUpdateTriggerTiming(def, paramChangeDict.at(name), totalMeasures, chartData, timingCache)
							: PrecalculateUpdateTriggerTiming(def, totalMeasures, chartData, timingCache);
						const auto paramChanges =
							paramChangeDict.contains(name)
							? ConvertParamChangesFromPulseToSec(paramChangeDict.at(name), chartData, timingCache)
							: std::unordered_map<std::string, std::map<float, std::string>>{};

						bgm.emplaceAudioEffectFX(name, def, paramChanges, updateTriggerTiming);
					}
				}
			}

			// LASER
			fnInsertDefaultLaser("peaking_filter", { .type = kson::AudioEffectType::PeakingFilter });
			fnInsertDefaultLaser("high_pass_filter", { .type = kson::AudioEffectType::HighPassFilter });
			fnInsertDefaultLaser("low_pass_filter", { .type = kson::AudioEffectType::LowPassFilter });
			fnInsertDefaultLaser("bitcrusher", { .type = kson::AudioEffectType::Bitcrusher });
			// ユーザー定義エフェクトを追加
			for (const auto& kvp : chartData.audio.audioEffect.laser.def)
			{
				defLaser.push_back(
					AudioEffectDefWithBuiltinFlag{
						.name = kvp.name,
						.def = kvp.v,
						.isBuiltin = false,
					});
			}

			// p,fp音源指定時はリアルタイムエフェクトなし
			const bool skipLaserEffects = legacyMode == LegacyAudioFPMode::kFP;
			if (!skipLaserEffects)
			{
				for (const auto& [name, def, isBuiltin] : defLaser)
				{
					if (def.type == kson::AudioEffectType::SwitchAudio)
					{
						// SwitchAudioの場合
						if (def.v.contains("filename"))
						{
							// ファイル名をもとに音声を追加ロード
							const std::string& filename = def.v.at("filename");
							bgm.emplaceSwitchAudioStream(false, name, filename, parentPath, bgmVolume);
						}
					}
					else
					{
						// 通常のDSPエフェクトの場合
						const std::set<float> updateTriggerTiming =
							laserParamChangeDict.contains(name)
							? PrecalculateUpdateTriggerTiming(def, laserParamChangeDict.at(name), totalMeasures, chartData, timingCache)
							: PrecalculateUpdateTriggerTiming(def, totalMeasures, chartData, timingCache);
						const auto paramChanges =
							laserParamChangeDict.contains(name)
							? ConvertParamChangesFromPulseToSec(laserParamChangeDict.at(name), chartData, timingCache)
							: std::unordered_map<std::string, std::map<float, std::string>>{};

						// メイン音源には常に登録
						bgm.emplaceAudioEffectLaser(name, def, paramChanges, updateTriggerTiming);

						// FXのSwitchAudioストリームには標準のLASERエフェクト(peak,hpf,lpf,bitc)のみを登録
						if (isBuiltin)
						{
							bgm.emplaceSwitchAudioLaserEffect(name, def, paramChanges, updateTriggerTiming);

							// レガシーf音源用のビルトインレーザーエフェクト登録(f音源のみ指定時)
							if (legacyMode == LegacyAudioFPMode::kF)
							{
								bgm.legacyAudioFPStream().emplaceAudioEffectLaser(name, def, paramChanges, updateTriggerTiming);
							}
						}
					}
				}
			}
		}

		kson::FXLane<Optional<AudioEffectInvocation>> CreateLongFXNoteAudioEffectInvocations(BGM& bgm, const kson::ChartData& chartData)
		{
			const auto& longEvent = chartData.audio.audioEffect.fx.longEvent;
			const auto& audioEffectBus = bgm.audioEffectBusFX();
			kson::FXLane<Optional<AudioEffectInvocation>> convertedLongEvent;
			for (const auto& [audioEffectName, lanes] : longEvent)
			{
				for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
				{
					for (const auto& [y, dict] : lanes[i])
					{
						if (audioEffectName.empty())
						{
							// エフェクトなしへの変更の場合はエフェクトなしを挿入
							convertedLongEvent[i].insert_or_assign(y, none);
							continue;
						}

						const auto switchAudioIdx = bgm.switchAudioIdxByNameFX(audioEffectName);
						if (switchAudioIdx.has_value())
						{
							// SwitchAudioの場合
							convertedLongEvent[i].insert_or_assign(y, SwitchAudioInvocation{
								.switchAudioIdx = switchAudioIdx.value(),
							});
						}
						else if (audioEffectBus.audioEffectContainsName(audioEffectName))
						{
							// 通常のDSPエフェクトの場合
							convertedLongEvent[i].insert_or_assign(y, DSPAudioEffectInvocation{
								.audioEffectIdx = audioEffectBus.audioEffectNameToIdx(audioEffectName),
								.overrideParams = ksmaudio::AudioEffect::StrDictToParamValueSetDict(dict),
							});
						}
					}
				}
			}
			return convertedLongEvent;
		}

		kson::ByPulse<Optional<AudioEffectInvocation>> CreateLaserPulseAudioEffectInvocations(BGM& bgm, const kson::ChartData& chartData)
		{
			const auto& audioEffectLaser = chartData.audio.audioEffect.laser;
			const auto fnIsPeakingFilter = [&audioEffectLaser](const std::string& audioEffectName) -> bool
			{
				if (audioEffectLaser.defContains(audioEffectName))
				{
					// ユーザー定義のpeaking_filterの場合
					return audioEffectLaser.defByName(audioEffectName).type == kson::AudioEffectType::PeakingFilter;
				}
				else
				{
					// 暗黙に定義されているpeaking_filterの場合
					return audioEffectName == kPeakingFilterAudioEffectName;
				}
			};
			const auto& audioEffectBus = bgm.audioEffectBusLaser();
			kson::ByPulse<Optional<AudioEffectInvocation>> convertedPulseEvent;
			for (const auto& [audioEffectName, pulses] : audioEffectLaser.pulseEvent)
			{
				for (const auto& y : pulses)
				{
					const auto switchAudioIdx = bgm.switchAudioIdxByNameLaser(audioEffectName);
					if (switchAudioIdx.has_value())
					{
						// SwitchAudioの場合
						convertedPulseEvent.insert_or_assign(y, SwitchAudioInvocation{
							.switchAudioIdx = switchAudioIdx.value(),
						});
					}
					else if (audioEffectBus.audioEffectContainsName(audioEffectName))
					{
						// 通常のDSPエフェクトの場合
						convertedPulseEvent.insert_or_assign(y, DSPAudioEffectInvocation{
							.audioEffectIdx = audioEffectBus.audioEffectNameToIdx(audioEffectName),
							.isPeakingFilterLaser = fnIsPeakingFilter(audioEffectName),
						});
					}
					else
					{
						// 存在しないエフェクトの場合はエフェクトなしにする
						convertedPulseEvent.insert_or_assign(y, none);
					}
				}
			}

			// LASERエフェクト呼び出しがない、または、初回のLASERエフェクト呼び出しがゼロ地点より後ろの場合は、ゼロ地点にデフォルト値を挿入
			if (convertedPulseEvent.empty() || convertedPulseEvent.begin()->first > kson::Pulse{ 0 })
			{
				const std::string defaultAudioEffectName{ kDefaultLaserAudioEffectName };
				if (audioEffectBus.audioEffectContainsName(defaultAudioEffectName))
				{
					convertedPulseEvent.insert_or_assign(kson::Pulse{ 0 }, DSPAudioEffectInvocation{
						.audioEffectIdx = audioEffectBus.audioEffectNameToIdx(defaultAudioEffectName),
						.isPeakingFilterLaser = fnIsPeakingFilter(defaultAudioEffectName),
					});
				}
				else
				{
					// LASERにデフォルト値の音声エフェクトが定義されていない場合はエフェクトなしにする
					convertedPulseEvent.insert_or_assign(kson::Pulse{ 0 }, none);
				}
			}

			return convertedPulseEvent;
		}

		Optional<std::pair<kson::Pulse, kson::Interval>> CurrentLongNoteByTime(const kson::ByPulse<kson::Interval>& lane, kson::Pulse currentPulse)
		{
			const auto currentNoteItr = kson::ValueItrAt(lane, currentPulse);
			if (currentNoteItr != lane.end())
			{
				const auto& [y, currentNote] = *currentNoteItr;
				if (y <= currentPulse && currentPulse < y + currentNote.length)
				{
					return *currentNoteItr;
				}
			}
			return none;
		}
	}

	void AudioEffectMain::updateActiveAudioEffectDictFX(
		const std::array<Optional<std::pair<kson::Pulse, kson::Interval>>, kson::kNumFXLanesSZ>& currentLongNoteOfLanes, kson::Pulse currentPulseForAudio)
	{
		m_activeAudioEffectDictFX.clear();

		// レーン毎に現在押されている最中のロングFXノーツの音声エフェクトを列挙
		for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
		{
			// 2レーンのうち直近ロングFXノーツが押され始めた方のレーンを先に処理する
			// (2レーンで互いに異なるパラメータ値を持つ同じ音声エフェクトが指定される場合があり、直近の方を優先する必要があるため。
			//  例: "wave_length=1/8"のRetriggerと"wave_length=1/16"のRetriggerが同時に配置されている場合、"wave_length"は後に押した方で指定された値にする必要がある)
			// なお, ActiveAudioEffectDict::emplace()では同じキーの2回目の挿入は無視される
			static_assert(kson::kNumFXLanesSZ == 2U); // 下のコードはkNumFXLanesSZが2である前提である
			const std::size_t laneIdx = (i == 0) ? m_lastPressedLongFXNoteLaneIdx : (1U - m_lastPressedLongFXNoteLaneIdx);
			assert(laneIdx < kson::kNumFXLanesSZ);

			if (!currentLongNoteOfLanes[laneIdx].has_value())
			{
				// ロングFXノーツを押していない場合はエフェクトなし
				continue;
			}

			// そのレーンの直近の音声エフェクト呼び出しを取得
			const auto& [longNoteY, longNote] = *currentLongNoteOfLanes[laneIdx];
			const auto itr = kson::ValueItrAt(m_longFXNoteInvocations[laneIdx], currentPulseForAudio);
			if (itr == m_longFXNoteInvocations[laneIdx].end())
			{
				// 見つからなかった場合はエフェクトなし
				continue;
			}

			// 取得した音声エフェクト呼び出しが現在押下中のロングノーツに属するかを調べる
			//
			// 1本のロングFXノーツの途中で他の種類の音声エフェクトに変更される場合がある。
			// そのため、イベントが現在のロングノーツに属するかを調べるには、ロングノーツの開始点との一致判定ではなく、ロングノーツの範囲(開始～終了)の中にあるかを調べる必要がある。
			const auto& [longEventY, audioEffectInvocationOpt] = *itr;
			if (!audioEffectInvocationOpt.has_value())
			{
				// エフェクトなしへの変更の場合はエフェクトなし
				continue;
			}
			const auto& audioEffectInvocation = *audioEffectInvocationOpt;
			if (longEventY > currentPulseForAudio || longEventY < longNoteY || longNoteY + longNote.length <= longEventY)
			{
				// 現在のロングノーツの範囲外にある場合はエフェクトなし
				continue;
			}

			// アクティブな音声エフェクト呼び出しとして追加
			// (SwitchAudioはupdate関数側で別途処理するためここではDSPエフェクトのみ対象とする)
			if (std::holds_alternative<DSPAudioEffectInvocation>(audioEffectInvocation))
			{
				const auto& dspInvocation = std::get<DSPAudioEffectInvocation>(audioEffectInvocation);
				m_activeAudioEffectDictFX.emplace(dspInvocation.audioEffectIdx, ksmaudio::AudioEffect::ActiveAudioEffectInvocation{
					.pOverrideParams = &dspInvocation.overrideParams,
					.laneIdx = laneIdx,
				});
			}
		}
	}

	const Optional<AudioEffectInvocation>& AudioEffectMain::getActiveLaserAudioEffectInvocation(kson::Pulse currentPulseForAudio) const
	{
		assert(!m_laserPulseInvocations.empty()); // ゼロ地点にデフォルト値が挿入されるのでここでは空ではないはず

		const auto& [y, audioEffectInvocation] = *kson::ValueItrAt(m_laserPulseInvocations, currentPulseForAudio);
		return audioEffectInvocation;
	}

	AudioEffectMain::AudioEffectMain(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache, const FilePath& parentPath, double audioProcDelaySec, double bgmVolume)
		: m_longFXNoteInvocations((RegisterAudioEffects(bgm, chartData, timingCache, parentPath, bgmVolume), CreateLongFXNoteAudioEffectInvocations(bgm, chartData))) // 先に登録しておく必要があるので、分かりにくいがカンマ演算子を使用している(TODO: もうちょっとどうにかする)
		, m_laserPulseInvocations(CreateLaserPulseAudioEffectInvocations(bgm, chartData))
		, m_audioProcDelaySec(audioProcDelaySec)
		, m_peakingFilterDelaySec(GetEffectivePeakingFilterDelaySec(chartData))
	{
	}

	void AudioEffectMain::update(BGM& bgm, const kson::ChartData& chartData, const kson::TimingCache& timingCache, const AudioEffectInputStatus& inputStatus, kson::Pulse currentPulseForSwitchAudio)
	{
		// TODO: SecondsFに統一
		const double currentTimeSec = bgm.posSec().count();
		const double currentTimeSecForAudio = (currentTimeSec - m_audioProcDelaySec) + bgm.latency().count(); // Note: In BASS v2.4.13 and later, for unknown reasons, the effects are out of sync even after adding this latency.
		const kson::Pulse currentPulseForAudio = kson::SecToPulse(currentTimeSecForAudio, chartData.beat, timingCache);
		const double currentBPMForAudio = kson::TempoAt(currentPulseForAudio, chartData.beat);

		std::array<Optional<std::pair<kson::Pulse, kson::Interval>>, kson::kNumFXLanesSZ> currentLongNoteOfLanes;
		bool bypassFX = true;

		// ロングFXノーツの音声エフェクト
		{
			for (std::size_t i = 0; i < kson::kNumFXLanesSZ; ++i)
			{
				const auto currentLongNoteByTime = CurrentLongNoteByTime(chartData.note.fx[i], currentPulseForAudio);

				// 音声エフェクトはバッファサイズによる遅延を回避するため、バッファサイズ分だけ早めに適用し始める必要がある。
				// これはプレイヤーのキー入力に関係なく実施する必要があるため、下記の「～.value_or(true)」で、longFXPressed[i]がnoneである(つまり判定中のロングFXノーツがまだ存在しない)場合でも、
				// あたかもロングFXが押されている(trueである)かのように扱っている。
				// 
				// それに加えて、キー入力がやや遅れた場合でも音声エフェクトが途切れないように、(バッファサイズとは別に)最初の30ms分はプレイヤーのキー入力に関係なく音声エフェクトを有効にしている。
				// 
				// HSP版での対応箇所: https://github.com/kshootmania/ksm-v1/blob/19bfb6acbec8abd304b2e7dae6009df8e8e1f66f/src/scene/play/play_audio_effects.hsp#L488
				if (currentLongNoteByTime.has_value()
					&& (inputStatus.longFXPressed[i].value_or(true)
						|| (currentTimeSec - kson::PulseToSec(currentLongNoteByTime->first, chartData.beat, timingCache)) < kLongFXNoteAudioEffectAutoPlaySec))
				{
					currentLongNoteOfLanes[i] = currentLongNoteByTime;
					bypassFX = false;
					if (!m_longFXPressedPrev[i])
					{
						m_lastPressedLongFXNoteLaneIdx = i;
						m_longFXPressedPrev[i] = true;
					}
				}
				else
				{
					currentLongNoteOfLanes[i] = none;
					m_longFXPressedPrev[i] = false;
				}
			}
			updateActiveAudioEffectDictFX(currentLongNoteOfLanes, currentPulseForAudio);
			bgm.updateAudioEffectFX(
				bypassFX,
				{
					.bpm = static_cast<float>(currentBPMForAudio),
					.sec = static_cast<float>(currentTimeSecForAudio),
					.playbackSpeed = static_cast<float>(bgm.playbackSpeed()),
				},
				m_activeAudioEffectDictFX);
		}

		const Optional<AudioEffectInvocation>& activeLaserInvocation = getActiveLaserAudioEffectInvocation(currentPulseForAudio);
		const Optional<AudioEffectInvocation>& activeLaserInvocationForSwitchAudio = getActiveLaserAudioEffectInvocation(currentPulseForSwitchAudio);
		bool bypassLaser = true;

		// LASERノーツの音声エフェクト
		{
			const bool isPeakingFilter = activeLaserInvocation.has_value()
				&& std::holds_alternative<DSPAudioEffectInvocation>(*activeLaserInvocation)
				&& std::get<DSPAudioEffectInvocation>(*activeLaserInvocation).isPeakingFilterLaser;
			kson::Pulse currentPulseForLaserAudio;
			if (isPeakingFilter && m_peakingFilterDelaySec != 0.0)
			{
				// peaking_filterの場合は遅延時間を適用
				const double currentTimeSecForLaserAudio = currentTimeSecForAudio - m_peakingFilterDelaySec;
				currentPulseForLaserAudio = kson::SecToPulse(currentTimeSecForLaserAudio, chartData.beat, timingCache);
			}
			else
			{
				// peaking_filterでない場合はそのままの時間を使用
				currentPulseForLaserAudio = currentPulseForAudio;
			}

			float laserValue = 0.0f;
			for (std::size_t i = 0; i < kson::kNumLaserLanesSZ; ++i)
			{
				if (!inputStatus.laserIsOnOrNone[i])
				{
					// LASER判定中でOffの状態の場合(カーソルがノーツから外れている場合)はエフェクトを適用しない
					continue;
				}

				const auto& laneLaserValue = kson::GraphSectionValueAt(chartData.note.laser[i], currentPulseForLaserAudio);
				if (!laneLaserValue.has_value())
				{
					// 現在時間(currentPulseForLaserAudio)の時点にLASERノーツがない場合はエフェクトを適用しない
					continue;
				}

				// 表示上のX座標の値(0.0:左、1.0:右)を、音声エフェクトで使うためのレーンに依存しない値(0.0～1.0)に変換
				const float fLaneLaserValue = static_cast<float>(laneLaserValue.value());
				static_assert(kson::kNumLaserLanesSZ == 2U); // 以下の処理はLASERのレーン数が2であることを前提にしている
				const float v = (i == 0U) ? fLaneLaserValue : (1.0f - fLaneLaserValue); // 右レーンの値を反転

				// 音声エフェクトにはLASERの左右レーンのうち値が大きい方を採用する
				if (laserValue < v)
				{
					laserValue = v;
				}
				bypassLaser = false;
			}
			bgm.updateAudioEffectLaser(
				bypassLaser,
				{
					.v = laserValue,
					.bpm = static_cast<float>(currentBPMForAudio),
					.sec = static_cast<float>(currentTimeSecForAudio),
					.playbackSpeed = static_cast<float>(bgm.playbackSpeed()),
				},
				activeLaserInvocation.has_value() && std::holds_alternative<DSPAudioEffectInvocation>(*activeLaserInvocation)
				? std::make_optional(std::get<DSPAudioEffectInvocation>(*activeLaserInvocation).audioEffectIdx)
				: std::nullopt);
		}

		Optional<std::size_t> activeSwitchAudioIdxFX;
		Optional<std::size_t> activeSwitchAudioIdxLaser;
	
		// FXのSwitchAudioチェック
		for (std::size_t i = 0U; i < kson::kNumFXLanesSZ; ++i)
		{
			// ボタンが実際に押されているかチェック(SwitchAudioは先行発音しないためbypassとは別に調べる)
			if (!inputStatus.longFXPressed[i].value_or(false))
			{
				continue;
			}

			const auto currentLongNoteByTime = CurrentLongNoteByTime(chartData.note.fx[i], currentPulseForSwitchAudio);
			if (!currentLongNoteByTime.has_value())
			{
				continue;
			}
			const auto& [longNoteY, longNote] = *currentLongNoteByTime;
			const auto itr = kson::ValueItrAt(m_longFXNoteInvocations[i], currentPulseForSwitchAudio);
			if (itr == m_longFXNoteInvocations[i].end())
			{
				continue;
			}
			const auto& [longEventY, audioEffectInvocationOpt] = *itr;
			if (!audioEffectInvocationOpt.has_value())
			{
				continue;
			}
			const auto& audioEffectInvocation = *audioEffectInvocationOpt;
			if (longEventY > currentPulseForSwitchAudio || longEventY < longNoteY || longNoteY + longNote.length <= longEventY)
			{
				continue;
			}
			if (std::holds_alternative<SwitchAudioInvocation>(audioEffectInvocation))
			{
				const auto& switchInvocation = std::get<SwitchAudioInvocation>(audioEffectInvocation);
				activeSwitchAudioIdxFX = switchInvocation.switchAudioIdx;
				break;
			}
		}
	
		// LASERのSwitchAudioチェック
		{
			// いずれかのレーザーがOnかつノーツがある状態か調べる(SwitchAudioは先行発音しないためbypassとは別に調べる)
			bool isAnyLaserOn = false;
			for (std::size_t i = 0; i < kson::kNumLaserLanesSZ; ++i)
			{
				if (inputStatus.laserIsOnOrNone[i])
				{
					const auto& laneLaserValue = kson::GraphSectionValueAt(chartData.note.laser[i], currentPulseForSwitchAudio);
					if (laneLaserValue.has_value())
					{
						isAnyLaserOn = true;
						break;
					}
				}
			}

			if (isAnyLaserOn && activeLaserInvocationForSwitchAudio.has_value() && std::holds_alternative<SwitchAudioInvocation>(*activeLaserInvocationForSwitchAudio))
			{
				const auto& switchInvocation = std::get<SwitchAudioInvocation>(*activeLaserInvocationForSwitchAudio);
				activeSwitchAudioIdxLaser = switchInvocation.switchAudioIdx;
			}
		}
	
		// f/fp音源の切り替え判定
		if (bgm.legacyAudioFPStream().mode != LegacyAudioFPMode::kNone)
		{
			// 両FXまたは両レーザーが同時に出ている場合は、両方が判定されている必要がある
			bool legacyFXActive = false;
			bool legacyLaserActive = false;

			// ロングFXが出ている全てのレーンで判定中の場合にf音源へ切り替え
			{
				int activeFXCount = 0;
				int totalFXCount = 0;
				for (std::size_t i = 0; i < kson::kNumFXLanesSZ; ++i)
				{
					const auto currentLongNoteByTime = CurrentLongNoteByTime(chartData.note.fx[i], currentPulseForSwitchAudio);
					if (currentLongNoteByTime.has_value())
					{
						totalFXCount++;
						if (inputStatus.longFXPressed[i].value_or(false))
						{
							activeFXCount++;
						}
					}
				}
				if (totalFXCount > 0 && activeFXCount == totalFXCount)
				{
					legacyFXActive = true;
				}
			}

			// LASERが出ている全てのレーンで判定中の場合にp音源へ切り替え
			{
				int activeLaserCount = 0;
				int totalLaserCount = 0;
				for (std::size_t i = 0; i < kson::kNumLaserLanesSZ; ++i)
				{
					const auto& laneLaserValue = kson::GraphSectionValueAt(chartData.note.laser[i], currentPulseForSwitchAudio);
					if (laneLaserValue.has_value())
					{
						totalLaserCount++;
						if (inputStatus.laserIsOnOrNone[i])
						{
							activeLaserCount++;
						}
					}
				}
				if (totalLaserCount > 0 && activeLaserCount == totalLaserCount)
				{
					legacyLaserActive = true;
				}
			}

			bgm.updateLegacyAudioMute(legacyFXActive, legacyLaserActive);
		}

		bgm.updateSwitchAudio(activeSwitchAudioIdxFX, activeSwitchAudioIdxLaser);
	}
}
