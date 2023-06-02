﻿#include "laser_lane_judgment.hpp"
#include "kson/util/timing_utils.hpp"
#include "kson/util/graph_utils.hpp"

namespace MusicGame::Judgment
{
	namespace
	{
		kson::ByPulse<int32> CreateLaserLineDirectionMap(const kson::ByPulse<kson::LaserSection>& lane)
		{
			kson::ByPulse<int32> directionMap;
			directionMap.emplace(kson::Pulse{ 0 }, 0);

			for (const auto& [y, sec] : lane)
			{
				kson::Pulse prevPulse = y;
				Optional<double> prevValue = none;
				for (const auto& [ry, v] : sec.v)
				{
					if (prevValue.has_value())
					{
						const int32 direction = Sign(v.v - prevValue.value());
						directionMap.insert_or_assign(prevPulse, direction);
					}
					prevPulse = y + ry;
					prevValue = v.vf;
				}
				directionMap.insert_or_assign(prevPulse, 0);
			}

			return directionMap;
		}

		kson::ByPulse<int32> CreateLaserLineDirectionMapForRippleEffect(const kson::ByPulse<int32>& laserLineDirectionMap, const kson::ByPulse<kson::LaserSection>& lane)
		{
			kson::ByPulse<int32> replacedDirectionMap;
			replacedDirectionMap.emplace(kson::Pulse{ 0 }, 0);

			// v1と挙動を同じにするため、エフェクト表示に使う方向については、方向0の扱いを「両端(0.0、1.0)は内側方向」「それ以外は前のLASERと同じ方向」とする
			// そのため、方向0の場合の値を読み替えたものを作成する
			int32 prevReplacedDirection = 0;
			for (const auto& [y, direction] : laserLineDirectionMap)
			{
				int32 replacedDirection = direction;
				if (direction == 0)
				{
					const auto pointOpt = kson::GraphPointAt(lane, y);
					if (!pointOpt.has_value())
					{
						// yが0の場合のみここを通り得る想定
						assert(y == 0 && "Out-of-range pulse detected in laserLineDirectionMap");
						continue;
					}
					const auto& point = pointOpt.value();
					if (MathUtils::AlmostEquals(point.vf, 0.0))
					{
						// 左端は右側方向として扱う
						replacedDirection = 1;
					}
					else if (MathUtils::AlmostEquals(point.vf, 1.0))
					{
						// 右端は左側方向として扱う
						replacedDirection = -1;
					}
					else
					{
						// それ以外は前のLASERと同じ方向として扱う
						replacedDirection = prevReplacedDirection;
					}
				}

				if (replacedDirection != prevReplacedDirection)
				{
					replacedDirectionMap.insert_or_assign(y, replacedDirection);
				}
			}

			return replacedDirectionMap;
		}

		Array<double> CreateLaserLineDirectionChangeSecArray(const kson::ByPulse<kson::LaserSection>& lane, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache)
		{
			Array<double> directionChangeSecArray;

			for (const auto& [y, sec] : lane)
			{
				kson::Pulse prevPulse = y;
				Optional<double> prevV = none;
				Optional<double> prevVf = none;
				int32 prevDirection = 0;
				for (const auto& [ry, v] : sec.v)
				{
					if (prevV.has_value() && prevVf.has_value())
					{
						const int32 direction = Sign(v.v - prevVf.value());
						if (direction != prevDirection || !kson::AlmostEquals(prevV.value(), prevVf.value()))
						{
							// 折り返しまたは直角があった場合はタイミングを配列に挿入
							directionChangeSecArray.push_back(kson::PulseToSec(prevPulse, beatInfo, timingCache));
							prevDirection = direction;
						}
					}
					prevPulse = y + ry;
					prevV = v.v;
					prevVf = v.vf;
				}

				// セクション末尾のタイミングも折り返しとして配列に挿入
				directionChangeSecArray.push_back(kson::PulseToSec(prevPulse, beatInfo, timingCache));
			}

			return directionChangeSecArray;
		}

		using LineJudgment = LaserLaneJudgment::LineJudgment;

		kson::ByPulse<LineJudgment> CreateLineJudgmentResultArray(const kson::ByPulse<kson::LaserSection>& lane, const kson::BeatInfo& beatInfo)
		{
			// button_lane_judgment.cppのCreateLongNoteJudgmentArray関数と同じやり方を採用

			kson::ByPulse<LineJudgment> judgmentArray;

			for (const auto& [y, section] : lane)
			{
				if (section.v.empty())
				{
					continue;
				}
				
				const kson::RelPulse sectionLength = section.v.rbegin()->first;
				if (sectionLength > 0)
				{
					// BPMをもとにLASERノーツのコンボ数を半減させるかを決める
					// (セクション途中でのBPM変更は特に加味しない)
					const bool halvesCombo = kson::TempoAt(y, beatInfo) >= kHalveComboBPMThreshold;
					const kson::RelPulse minPulseInterval = halvesCombo ? (kson::kResolution4 * 3 / 8) : (kson::kResolution4 * 3 / 16);
					const kson::RelPulse pulseInterval = halvesCombo ? (kson::kResolution4 / 8) : (kson::kResolution4 / 16);

					if (sectionLength < pulseInterval * 2)
					{
						judgmentArray.emplace(y, LineJudgment{ .length = sectionLength });
					}
					else if (sectionLength <= minPulseInterval)
					{
						judgmentArray.emplace(y + pulseInterval, LineJudgment{ .length = pulseInterval });
					}
					else
					{
						const kson::Pulse start = ((y + pulseInterval - 1) / pulseInterval + 1) * pulseInterval;
						const kson::Pulse end = y + sectionLength - pulseInterval;

						for (kson::Pulse pulse = start; pulse < end; pulse += pulseInterval)
						{
							const kson::RelPulse length = (pulse <= end - pulseInterval) ? pulseInterval : (pulseInterval * 2); // 末尾の判定のみ2倍の長さ
							judgmentArray.emplace(pulse, LineJudgment{ .length = length });
						}
					}
				}
			}

			return judgmentArray;
		}

		kson::ByPulse<LaserSlamJudgment> CreateSlamJudgmentArray(const kson::ByPulse<kson::LaserSection>& lane, const kson::BeatInfo& beatInfo, const kson::CamPatternLaserInvokeList& camPatternLaserInvokeList, const kson::TimingCache& timingCache)
		{
			kson::ByPulse<LaserSlamJudgment> judgmentArray;

			for (const auto& [y, section] : lane)
			{
				for (const auto& [ry, point] : section.v)
				{
					const bool isSlam = !kson::AlmostEquals(point.v, point.vf);
					if (isSlam)
					{
						const double slamTimeSec = kson::PulseToSec(y + ry, beatInfo, timingCache);
						const int32 slamDirection = Sign(point.vf - point.v);
						const Optional<kson::CamPatternInvokeSpin> invokeSpin = camPatternLaserInvokeList.spin.contains(y + ry) ? MakeOptional(camPatternLaserInvokeList.spin.at(y + ry)) : none;
						judgmentArray.emplace(y + ry, LaserSlamJudgment{ slamTimeSec, slamDirection, invokeSpin });
					}
				}
			}

			return judgmentArray;
		}

		Optional<std::pair<double, bool>> GetPregeneratedCursorValue(const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse)
		{
			// 直近1小節以内にレーザーセクションの始点が存在すればカーソルが出ている
			const auto itr = kson::FirstInRange(lane, currentPulse, currentPulse + kson::kResolution4);
			if (itr != lane.end())
			{
				const auto& [_, sec] = *itr;
				if (!sec.v.empty())
				{
					const auto& [_, v] = *sec.v.begin();
					return std::pair<double, bool>{ v.v, sec.wide() };
				}
			}

			return none;
		}
	}

	LaserSlamJudgment::LaserSlamJudgment(double sec, int32 direction, const Optional<kson::CamPatternInvokeSpin>& invokeSpin)
		: m_sec(sec)
		, m_direction(direction)
		, m_invokeSpin(invokeSpin)
	{
	}

	double LaserSlamJudgment::sec() const
	{
		return m_sec;
	}

	int32 LaserSlamJudgment::direction() const
	{
		return m_direction;
	}

	void LaserSlamJudgment::addDeltaCursorX(double deltaCursorX, double currentTimeSec)
	{
		if (Abs(currentTimeSec - m_sec) > TimingWindow::LaserNote::kWindowSecSlam)
		{
			// 判定時間外の場合は何もしない
			return;
		}

		if (isCriticalSatisfied())
		{
			// 既にCRITICAL判定に必要なカーソル移動量を満たしている場合は何もしない
			return;
		}

		if (Sign(deltaCursorX) == m_direction)
		{
			// 直角LASERと同方向の入力であれば、累計移動量に加算
			m_totalAbsDeltaCursorX += Abs(deltaCursorX);
		}
		else
		{
			// 直角LASERと逆方向の入力であれば、累計移動量から減算
			// (ただし、既に加算されたものを打ち消すだけで、0未満にはしない)
			m_totalAbsDeltaCursorX = Max(m_totalAbsDeltaCursorX - Abs(deltaCursorX), 0.0);
		}
	}

	bool LaserSlamJudgment::isCriticalSatisfied() const
	{
		return m_totalAbsDeltaCursorX >= kLaserSlamCriticalDeltaCursorXThreshold;
	}

	JudgmentResult LaserSlamJudgment::judgmentResult(double currentTimeSec) const
	{
		if (isCriticalSatisfied())
		{
			// CRITICAL判定の移動量を満たしていればCRITICAL判定
			return JudgmentResult::kCritical;
		}
		else if (currentTimeSec <= m_sec + TimingWindow::LaserNote::kWindowSecSlam)
		{
			// CRITICAL判定の移動量を満たしておらず、まだ判定時間終了前の場合は未判定
			return JudgmentResult::kUnspecified;
		}
		else
		{
			// CRITICAL判定の移動量を満たしておらず、既に判定時間を過ぎている場合はERROR判定
			return JudgmentResult::kError;
		}
	}

	void LaserLaneJudgment::processCursorMovement(double deltaCursorX, kson::Pulse currentPulse, double currentTimeSec, LaserLaneStatus& laneStatusRef)
	{
		if (!laneStatusRef.cursorX.has_value())
		{
			// カーソルが出ていない場合は何もしない
			return;
		}

		if (!laneStatusRef.noteCursorX.has_value())
		{
			// 事前生成されたカーソルは動かさない(いわゆる始点ロック)
			return;
		}

		const int32 direction = Sign(deltaCursorX);
		if (direction == 0)
		{
			// 移動方向がない場合は何もしない
			return;
		}
		const int32 noteDirection = kson::ValueItrAt(m_laserLineDirectionMap, currentPulse)->second;
		const double noteCursorX = laneStatusRef.noteCursorX.value();
		const double cursorX = laneStatusRef.cursorX.value();
		double nextCursorX;
		if (direction == noteDirection || noteDirection == 0)
		{
			// LASERノーツと同方向にカーソル移動している、または、LASERノーツが横移動なしの場合
			const double overshootCursorX = cursorX + deltaCursorX * kLaserCursorInputOvershootScale; // 増幅移動量で計算したカーソル移動先
			if (Min(cursorX, overshootCursorX) - kLaserAutoFitMaxDeltaCursorX < noteCursorX && noteCursorX < Max(cursorX, overshootCursorX) + kLaserAutoFitMaxDeltaCursorX)
			{
				// 増幅移動量で計算したカーソル移動の範囲内に理想位置があれば、カーソルを理想位置へ吸い付かせる
				nextCursorX = noteCursorX;
				m_lastCorrectMovementSec = currentTimeSec;
			}
			else
			{
				// 増幅移動量で理想位置に届かなければ、カーソルを単純に動かす
				nextCursorX = cursorX + deltaCursorX;
			}
		}
		else if (Abs(cursorX - noteCursorX) < kLaserAutoFitMaxDeltaCursorX)
		{
			// LASERカーソルが理想位置に近い場合はカーソルを逆方向に動かさない
			nextCursorX = cursorX;
		}
		else
		{
			// LASERノーツと逆方向にカーソル移動している場合、カーソルを単純に動かす
			nextCursorX = cursorX + deltaCursorX;
		}
		laneStatusRef.cursorX = Clamp(nextCursorX, 0.0, 1.0);
	}

	void LaserLaneJudgment::processSlamJudgment(const kson::ByPulse<kson::LaserSection>& lane, double deltaCursorX, double currentTimeSec, LaserLaneStatus& laneStatusRef, ScoringStatus& scoringStatusRef, ViewStatus& viewStatusRef)
	{
		// 直角LASERはまだカーソルが出ていなくても先行判定するので、カーソルの存在チェックはしない

		if (m_slamJudgmentArrayCursor == m_slamJudgmentArray.end())
		{
			// 残りの直角LASER判定がない場合は何もしない
			return;
		}

		// Note:
		// 1フレーム内では1つの直角LASERまでしか判定しない仕組みとしている
		// (1フレーム内で1つの直角LASER判定に必要なカーソル移動量を超える入力があったとしても、超えた分の移動量は特に加味せず破棄する)

		// 移動量を現在判定対象になっている直角LASERへ反映
		auto& [laserSlamPulse, laserSlamJudgmentRef] = *m_slamJudgmentArrayCursor;
		laserSlamJudgmentRef.addDeltaCursorX(deltaCursorX, currentTimeSec);

		// 判定が決まった場合
		const JudgmentResult judgmentResult = laserSlamJudgmentRef.judgmentResult(currentTimeSec);
		if (judgmentResult != JudgmentResult::kUnspecified)
		{
			scoringStatusRef.doChipJudgment(judgmentResult);

			if (judgmentResult == JudgmentResult::kCritical)
			{
				// 判定した時間を記録(補正および効果音再生に使用)
				laneStatusRef.lastLaserSlamJudgedTimeSec = Max(currentTimeSec, laserSlamJudgmentRef.sec());
				laneStatusRef.lastJudgedLaserSlamPulse = laserSlamPulse;

				// 直角LASER判定後の振動
				viewStatusRef.laserSlamWiggleStatus.onLaserSlamJudged(m_prevTimeSec, laserSlamJudgmentRef.direction());

				// アニメーション
				const auto sectionItr = kson::GraphSectionAt(lane, laserSlamPulse);
				assert(sectionItr != lane.end() && "Cannot find laser section of slam note");
				const auto& [y, section] = *sectionItr;
				const auto& point = section.v.at(laserSlamPulse - y);
				laneStatusRef.rippleAnim.push({
					.startTimeSec = Max(laserSlamJudgmentRef.sec(), currentTimeSec),
					.wide = section.wide(),
					.x = point.vf,
				});
			}
			else
			{
				// 直角LASERがERROR判定になった場合は補正を切る
				m_lastCorrectMovementSec = kPastTimeSec;
			}

			// 判定対象を次の直角LASERへ進める
			// (1フレームで複数の判定が過ぎ去っている可能性があるのでwhileで回す)
			while (true)
			{
				++m_slamJudgmentArrayCursor;

				// 末尾まで到達したら抜ける
				if (m_slamJudgmentArrayCursor == m_slamJudgmentArray.end())
				{
					break;
				}

				// 未判定である(まだERROR判定になっていない)直角LASERが見つかったら抜ける
				const auto& [_, nextLaserSlamJudgmentRef] = *m_slamJudgmentArrayCursor;
				if (nextLaserSlamJudgmentRef.judgmentResult(currentTimeSec) == JudgmentResult::kUnspecified)
				{
					break;
				}
			}
		}
	}

	void LaserLaneJudgment::processAutoCursorMovementBySlamJudgment(double currentTimeSec, LaserLaneStatus& laneStatusRef)
	{
		if (!laneStatusRef.cursorX.has_value())
		{
			// カーソルが出ていない場合は何もしない
			return;
		}

		if (!laneStatusRef.noteCursorX.has_value())
		{
			// 事前生成されたカーソルは動かさない(いわゆる始点ロック)
			return;
		}

		if (!laneStatusRef.currentLaserSectionPulse.has_value())
		{
			// noteCursorXが存在すればcurrentLaserSectionPulseも存在するはず
			assert(false && "currentLaserSectionPulse is none although noteCursorX is not none");
			return;
		}

		if (laneStatusRef.currentLaserSectionPulse.value() > laneStatusRef.lastJudgedLaserSlamPulse)
		{
			// 判定した直角LASERより後ろのLASERセクションを判定中の場合は補正しない
			return;
		}

		const double lastLaserSlamJudgedTimeSec = laneStatusRef.lastLaserSlamJudgedTimeSec;
		if (lastLaserSlamJudgedTimeSec <= currentTimeSec && currentTimeSec < lastLaserSlamJudgedTimeSec + kLaserAutoSecAfterSlamJudgment)
		{
			// 直角判定後の補正時間内であれば、カーソルを理想位置へ吸い付かせる
			laneStatusRef.cursorX = laneStatusRef.noteCursorX;
		}
	}

	void LaserLaneJudgment::processAutoCursorMovementByLineDirectionChange(double currentTimeSec, LaserLaneStatus& laneStatusRef)
	{
		if (!laneStatusRef.cursorX.has_value())
		{
			// カーソルが出ていない場合は何もしない
			return;
		}

		if (!laneStatusRef.noteCursorX.has_value())
		{
			// 事前生成されたカーソルは動かさない(いわゆる始点ロック)
			return;
		}

		// 折り返しタイミング周辺の時間範囲内にあるかを調べる
		for (; m_laserLineDirectionChangeSecArrayCursor != m_laserLineDirectionChangeSecArray.end(); ++m_laserLineDirectionChangeSecArrayCursor)
		{
			const double directionChangeSec = *m_laserLineDirectionChangeSecArrayCursor;
			if (currentTimeSec <= directionChangeSec - kLaserAutoSecBeforeLineDirectionChange && m_lastCorrectMovementSec <= directionChangeSec - kLaserAutoSecBeforeLineDirectionChangeByCorrectMovement)
			{
				// 次の補正タイミングが現在時間より後ろにあるので何もせず抜ける
				return;
			}
			else if (currentTimeSec < directionChangeSec + kLaserAutoSecAfterLineDirectionChange)
			{
				// 補正時間内なので補正を実行
				break;
			}
		}

		// 未判定のまま過ぎ去った直角LASERがある場合は補正を無視
		// (直角LASERを判定していないのに補正でカーソルが理想位置に飛ぶのを防ぐため)
		if (m_slamJudgmentArrayCursor != m_slamJudgmentArray.end())
		{
			const auto& [_, laserSlamJudgment] = *m_slamJudgmentArrayCursor;
			const double laserSlamSec = laserSlamJudgment.sec();
			if (laserSlamSec <= currentTimeSec)
			{
				if (m_laserLineDirectionChangeSecArrayCursor != m_laserLineDirectionChangeSecArray.end())
				{
					++m_laserLineDirectionChangeSecArrayCursor;
				}
				return;
			}
		}

		// 折り返し前後の補正時間内なので、カーソルが理想位置に近い場合は理想位置へ吸い付かせる
		const double cursorX = laneStatusRef.cursorX.value();
		const double noteCursorX = laneStatusRef.noteCursorX.value();
		if (MusicGame::Judgment::IsLaserCursorInAutoFitRange(cursorX, noteCursorX) || m_prevIsCursorInAutoFitRange)
		{
			laneStatusRef.cursorX = laneStatusRef.noteCursorX;
		}
	}

	void LaserLaneJudgment::processAutoCursorMovementAfterCorrectMovement(double currentTimeSec, LaserLaneStatus& laneStatusRef)
	{
		if (!laneStatusRef.cursorX.has_value())
		{
			// カーソルが出ていない場合は何もしない
			return;
		}

		if (!laneStatusRef.noteCursorX.has_value())
		{
			// 事前生成されたカーソルは動かさない(いわゆる始点ロック)
			return;
		}

		// 未判定のまま過ぎ去った直角LASERがある場合は補正を無視
		// (直角LASERを判定していないのに補正でカーソルが理想位置に飛ぶのを防ぐため)
		if (m_slamJudgmentArrayCursor != m_slamJudgmentArray.end())
		{
			const auto& [_, laserSlamJudgment] = *m_slamJudgmentArrayCursor;
			const double laserSlamSec = laserSlamJudgment.sec();
			if (laserSlamSec <= currentTimeSec)
			{
				m_lastCorrectMovementSec = kPastTimeSec;
				return;
			}
		}

		if (currentTimeSec - m_lastCorrectMovementSec >= kLaserAutoSecAfterCorrectMovement)
		{
			// 補正時間が過ぎているので何もしない
			return;
		}

		// 補正時間内なので、カーソルが理想位置に近い場合は理想位置へ吸い付かせる
		const double cursorX = laneStatusRef.cursorX.value();
		const double noteCursorX = laneStatusRef.noteCursorX.value();
		if (MusicGame::Judgment::IsLaserCursorInAutoFitRange(cursorX, noteCursorX) || m_prevIsCursorInAutoFitRange)
		{
			laneStatusRef.cursorX = laneStatusRef.noteCursorX;
		}
	}

	void LaserLaneJudgment::processLineJudgment(const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse, double currentTimeSec, LaserLaneStatus& laneStatusRef, ScoringStatus& scoringStatusRef)
	{
		if (!laneStatusRef.cursorX.has_value())
		{
			// カーソルが出ていない場合は何もしない
			return;
		}

		if (!laneStatusRef.noteCursorX.has_value() || !laneStatusRef.currentLaserSectionPulse.has_value())
		{
			// ノーツ判定中でない場合は何もしない
			// TODO: LASERセクション終了フレームで処理落ちした場合を考慮すると、前回フレームでノーツ判定中だった場合は判定した方が良い？
			return;
		}

		const kson::Pulse sectionStartPulse = laneStatusRef.currentLaserSectionPulse.value();
		const auto& sectionPoints = lane.at(sectionStartPulse).v;
		assert(!sectionPoints.empty() && "Laser section must not be empty");
		const kson::Pulse sectionEndPulse = sectionStartPulse + sectionPoints.rbegin()->first;
		const kson::Pulse limitPulse = Min(currentPulse + kson::RelPulse{ 1 }, sectionEndPulse);

		const double cursorX = laneStatusRef.cursorX.value();
		const double noteCursorX = laneStatusRef.noteCursorX.value();
		const JudgmentResult currentResult = IsLaserCursorInCriticalJudgmentRange(cursorX, noteCursorX) ? JudgmentResult::kCritical : JudgmentResult::kError;
		for (auto itr = m_lineJudgmentArray.upper_bound(sectionStartPulse - 1); itr != m_lineJudgmentArray.end(); ++itr)
		{
			auto& [y, judgment] = *itr;
			if (y + judgment.length <= m_prevPulse)
			{
				continue;
			}

			if (y >= limitPulse)
			{
				break;
			}

			if (judgment.result != JudgmentResult::kUnspecified)
			{
				// 既に判定が決まっている場合はスキップ
				continue;
			}

			judgment.result = currentResult;
			scoringStatusRef.doLongJudgment(currentResult);
		}
	}

	void LaserLaneJudgment::processPassedLineJudgment(const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse, LaserLaneStatus& laneStatusRef, ScoringStatus& scoringStatusRef)
	{
		for (auto itr = m_passedLineJudgmentCursor; itr != m_lineJudgmentArray.end(); ++itr)
		{
			auto& [y, judgment] = *itr;
			if (y + judgment.length < currentPulse)
			{
				// 通過済みロングノーツのERROR判定
				if (judgment.result == JudgmentResult::kUnspecified)
				{
					judgment.result = JudgmentResult::kError;
					scoringStatusRef.doLongJudgment(JudgmentResult::kError);
				}

				m_passedLineJudgmentCursor = std::next(itr);
			}
		}
	}

	LaserLaneJudgment::LaserLaneJudgment(KeyConfig::Button keyConfigButtonL, KeyConfig::Button keyConfigButtonR, const kson::ByPulse<kson::LaserSection>& lane, const kson::BeatInfo& beatInfo, const kson::CamPatternLaserInvokeList& camPatternLaserInvokeList, const kson::TimingCache& timingCache)
		: m_keyConfigButtonL(keyConfigButtonL)
		, m_keyConfigButtonR(keyConfigButtonR)
		, m_laserLineDirectionMap(CreateLaserLineDirectionMap(lane))
		, m_laserLineDirectionMapForRippleEffect(CreateLaserLineDirectionMapForRippleEffect(m_laserLineDirectionMap, lane))
		, m_laserLineDirectionChangeSecArray(CreateLaserLineDirectionChangeSecArray(lane, beatInfo, timingCache))
		, m_laserLineDirectionChangeSecArrayCursor(m_laserLineDirectionChangeSecArray.begin())
		, m_lineJudgmentArray(CreateLineJudgmentResultArray(lane, beatInfo))
		, m_passedLineJudgmentCursor(m_lineJudgmentArray.begin())
		, m_slamJudgmentArray(CreateSlamJudgmentArray(lane, beatInfo, camPatternLaserInvokeList, timingCache))
		, m_slamJudgmentArrayCursor(m_slamJudgmentArray.begin())
	{
	}

	void LaserLaneJudgment::update(const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse, double currentTimeSec, LaserLaneStatus& laneStatusRef, ScoringStatus& scoringStatusRef, ViewStatus& viewStatusRef)
	{
		laneStatusRef.noteCursorX = kson::GraphSectionValueAt(lane, currentPulse);
		laneStatusRef.noteVisualCursorX = laneStatusRef.noteCursorX; // TODO: タイミング調整に合わせてずらして取得

		// 現在判定対象になっているLASERセクションの始点Pulse値を取得
		if (laneStatusRef.noteCursorX.has_value())
		{
			const auto& [y, laserSection] = *kson::ValueItrAt(lane, currentPulse);
			laneStatusRef.currentLaserSectionPulse = y;
		}
		else
		{
			laneStatusRef.currentLaserSectionPulse = none;
		}

		const auto pregeneratedCursorValue = GetPregeneratedCursorValue(lane, currentPulse);
		const bool hasSectionChanged = laneStatusRef.currentLaserSectionPulse != m_prevCurrentLaserSectionPulse;
		if (hasSectionChanged)
		{
			// LASERノーツ終点後のアニメーション
			if (m_prevIsCursorInCriticalJudgmentRange && m_prevCurrentLaserSectionPulse.has_value() && !laneStatusRef.currentLaserSectionPulse.has_value())
			{
				const auto& section = lane.at(m_prevCurrentLaserSectionPulse.value());
				assert(!section.v.empty() && "Laser section must not be empty");
				const auto& [lastPointRy, lastPoint] = *section.v.rbegin();
				const bool isLastPointSlam = !MathUtils::AlmostEquals(lastPoint.v, lastPoint.vf);
				if (!isLastPointSlam) // 最後の点が直角LASERの場合、直角判定で別途アニメーションされるのでここでは不要
				{
					laneStatusRef.rippleAnim.push({
						.startTimeSec = currentTimeSec,
						.wide = section.wide(),
						.x = lastPoint.vf,
					});
				}
			}

			// 異なるLASERセクションに突入した初回フレームの場合、前回フレームでの判定状況を加味しない
			m_prevIsCursorInAutoFitRange = false;
			m_prevIsCursorInCriticalJudgmentRange = false;
		}
		if (laneStatusRef.noteCursorX.has_value())
		{
			// LASERセクションに突入した初回フレーム、または前回とは異なるLASERセクションに突入した場合
			if (!laneStatusRef.cursorX.has_value() || hasSectionChanged)
			{
				// カーソルを出現させ、LASERセクションの始点の値に合わせる
				// (※現在の理想位置に合わせるのではない理由は、始点が直角の場合に直角の移動先にカーソルが合ってしまうため)
				const auto& laserSection = lane.at(laneStatusRef.currentLaserSectionPulse.value());
				laneStatusRef.cursorX = laserSection.v.begin()->second.v;
				laneStatusRef.cursorWide = laserSection.wide();
			}
		}
		else
		{
			if (pregeneratedCursorValue.has_value())
			{
				// カーソルをLASERノーツの手前で事前生成し、次のLASERセクションの始点の値に合わせる
				const auto& [cursorX, cursorWide] = pregeneratedCursorValue.value();
				laneStatusRef.cursorX = cursorX;
				laneStatusRef.cursorWide = cursorWide;
			}
			else
			{
				// カーソルが消滅
				laneStatusRef.cursorX = none;
				laneStatusRef.cursorWide = false;
			}

			// 補正時間をリセット
			m_lastCorrectMovementSec = kPastTimeSec;
		}

		// キー押下中の判定処理
		// (左向きキーと右向きキーを同時に押している場合、最後に押した方を優先する)
		const Optional<KeyConfig::Button> lastPressedButton = KeyConfig::LastPressedLaserButton(m_keyConfigButtonL, m_keyConfigButtonR);
		double deltaCursorX;
		if (lastPressedButton.has_value())
		{
			const int32 direction = lastPressedButton == m_keyConfigButtonL ? -1 : 1;
			deltaCursorX = kLaserKeyboardCursorXPerSec * Scene::DeltaTime() * direction; // TODO: Scene::DeltaTime()を使わずcurrentTimeSecの差分を使う
		}
		else
		{
			deltaCursorX = 0.0;
		}
		processCursorMovement(deltaCursorX, currentPulse, currentTimeSec, laneStatusRef);
		processSlamJudgment(lane, deltaCursorX, currentTimeSec, laneStatusRef, scoringStatusRef, viewStatusRef);

		// 直角LASER判定直後のカーソル自動移動
		processAutoCursorMovementBySlamJudgment(currentTimeSec, laneStatusRef);

		// 折り返し地点でのカーソル自動移動
		processAutoCursorMovementByLineDirectionChange(currentTimeSec, laneStatusRef);

		// 正解判定後のカーソル自動移動
		processAutoCursorMovementAfterCorrectMovement(currentTimeSec, laneStatusRef);

		// カーソル位置をもとにLASERの判定を決定
		processLineJudgment(lane, currentPulse, currentTimeSec, laneStatusRef, scoringStatusRef);

		// 通過済みのLASER判定をERRORにする
		processPassedLineJudgment(lane, currentPulse, laneStatusRef, scoringStatusRef);

		if (m_prevIsCursorInCriticalJudgmentRange)
		{
			const int32 direction = kson::ValueItrAt(m_laserLineDirectionMapForRippleEffect, currentPulse)->second;
			const int32 prevDirection = kson::ValueItrAt(m_laserLineDirectionMapForRippleEffect, m_prevPulse)->second;
			if (laneStatusRef.cursorX.has_value())
			{
				// 折り返し時のアニメーション
				if (direction != prevDirection && prevDirection != 0)
				{
					laneStatusRef.rippleAnim.push({
						.startTimeSec = currentTimeSec,
						.wide = laneStatusRef.cursorWide,
						.x = laneStatusRef.cursorX.value(),
					});
				}
			}
		}

		m_prevCurrentLaserSectionPulse = laneStatusRef.currentLaserSectionPulse;

		m_prevIsCursorInAutoFitRange =
			laneStatusRef.cursorX.has_value() &&
			laneStatusRef.noteCursorX.has_value() &&
			IsLaserCursorInAutoFitRange(laneStatusRef.cursorX.value(), laneStatusRef.noteCursorX.value());
		m_prevIsCursorInCriticalJudgmentRange = laneStatusRef.isCursorInCriticalJudgmentRange();

		m_prevPulse = currentPulse;
		m_prevTimeSec = currentTimeSec;
	}

	std::size_t LaserLaneJudgment::lineJudgmentCount() const
	{
		return m_lineJudgmentArray.size();
	}

	std::size_t LaserLaneJudgment::slamJudgmentCount() const
	{
		return m_slamJudgmentArray.size();
	}
}
