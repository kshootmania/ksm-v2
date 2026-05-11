#include "LaserLaneJudgment.hpp"
#include "kson/Util/TimingUtils.hpp"
#include "kson/Util/GraphUtils.hpp"
#include "Input/KeyConfig.hpp"

namespace MusicGame::Judgment
{
	void LaserInputAccumulator::addDeltaCursorX(double deltaCursorX)
	{
		m_accumulatedDeltaCursorX += deltaCursorX;
	}

	bool LaserInputAccumulator::shouldApplyAmplification(double currentTimeSec) const
	{
		return currentTimeSec - m_lastCheckTimeSec >= kLaserInputAmplificationCheckIntervalSec;
	}

	double LaserInputAccumulator::getAccumulatedDeltaCursorX() const
	{
		return m_accumulatedDeltaCursorX;
	}

	void LaserInputAccumulator::resetAccumulation(double currentTimeSec)
	{
		m_lastCheckTimeSec = currentTimeSec;
		m_accumulatedDeltaCursorX = 0.0;
	}

	namespace
	{
		kson::ByPulse<int32> CreateLaserLineDirectionMap(const kson::ByPulse<kson::LaserSection>& lane)
		{
			kson::ByPulse<int32> directionMap;
			directionMap.emplace(kson::Pulse{ 0 }, 0);

			for (const auto& [y, section] : lane)
			{
				kson::Pulse prevPulse = y;
				Optional<double> prevValue = none;
				for (const auto& [ry, point] : section.v)
				{
					if (prevValue.has_value())
					{
						const int32 direction = Sign(point.v.v - prevValue.value());
						directionMap.insert_or_assign(prevPulse, direction);
					}
					prevPulse = y + ry;
					prevValue = point.v.vf;
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
					if (MathUtils::AlmostEquals(point.v.vf, 0.0))
					{
						// 左端は右側方向として扱う
						replacedDirection = 1;
					}
					else if (MathUtils::AlmostEquals(point.v.vf, 1.0))
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
				for (const auto& [ry, point] : sec.v)
				{
					if (prevV.has_value() && prevVf.has_value())
					{
						const int32 direction = Sign(point.v.v - prevVf.value());
						if (direction != prevDirection || !kson::AlmostEquals(prevV.value(), prevVf.value()))
						{
							// 折り返しまたは直角があった場合はタイミングを配列に挿入
							directionChangeSecArray.push_back(kson::PulseToSec(prevPulse, beatInfo, timingCache));
							prevDirection = direction;
						}
					}
					prevPulse = y + ry;
					prevV = point.v.v;
					prevVf = point.v.vf;
				}

				// セクション末尾のタイミングも折り返しとして配列に挿入
				directionChangeSecArray.push_back(kson::PulseToSec(prevPulse, beatInfo, timingCache));
			}

			return directionChangeSecArray;
		}

		using LineJudgment = LaserLaneJudgment::LineJudgment;

		kson::ByPulse<LineJudgment> CreateLineJudgmentResultArray(const kson::ByPulse<kson::LaserSection>& lane, const kson::BeatInfo& beatInfo, JudgmentPlayMode judgmentPlayMode)
		{
			// Offモードの場合は空配列を返す
			if (judgmentPlayMode == JudgmentPlayMode::kOff)
			{
				return {};
			}

			kson::ByPulse<LineJudgment> judgmentArray;

			for (const auto& [y, section] : lane)
			{
				if (section.v.empty())
				{
					continue;
				}

				// LASERセクション内のセグメントごとにライン判定を生成
				auto itr = section.v.cbegin();
				while (itr != section.v.cend())
				{
					const auto& [ry, point] = *itr;
					auto nextItr = std::next(itr);
					if (nextItr == section.v.cend())
					{
						break;
					}
					const auto& [nextRy, nextPoint] = *nextItr;

					kson::Pulse segStartAbs = y + ry;
					const kson::Pulse segEndAbs = y + nextRy;

					const bool isSlam = !kson::AlmostEquals(point.v.v, point.v.vf);
					if (isSlam)
					{
						// 直角LASERから次の点までが32分以内ならライン判定なし
						const bool nextIsSlamDst = kson::AlmostEquals(nextPoint.v.v, point.v.vf) &&
							nextRy - ry <= kson::kResolution4 / 32;
						if (nextIsSlamDst)
						{
							++itr;
							continue;
						}

						// 直角LASERから32分の範囲はライン判定から除外
						segStartAbs += kson::kResolution4 / 32;
						if (segStartAbs >= segEndAbs)
						{
							++itr;
							continue;
						}
					}

					// BPMをもとにLASERノーツのコンボ数を半減させるかを決める
					// (セグメントの途中でのBPM変更は特に加味しない)
					const bool halvesCombo = kson::TempoAt(segStartAbs, beatInfo) >= kHalveComboBPMThreshold;
					const kson::RelPulse pulseInterval = halvesCombo ? kson::kResolution4 / 8 : kson::kResolution4 / 16;
					const kson::Pulse start = (segStartAbs + pulseInterval - 1) / pulseInterval * pulseInterval;
					for (kson::Pulse pulse = start; pulse < segEndAbs; pulse += pulseInterval)
					{
						judgmentArray.emplace(pulse, LineJudgment{ .length = pulseInterval });
					}

					++itr;
				}
			}

			return judgmentArray;
		}

		kson::ByPulse<LaserSlamJudgment> CreateSlamJudgmentArray(const kson::ByPulse<kson::LaserSection>& lane, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache, JudgmentPlayMode judgmentPlayMode)
		{
			// Offモードの場合は空配列を返す
			if (judgmentPlayMode == JudgmentPlayMode::kOff)
			{
				return {};
			}

			kson::ByPulse<LaserSlamJudgment> judgmentArray;

			for (const auto& [y, section] : lane)
			{
				for (const auto& [ry, point] : section.v)
				{
					const bool isSlam = !kson::AlmostEquals(point.v.v, point.v.vf);
					if (isSlam)
					{
						const double slamTimeSec = kson::PulseToSec(y + ry, beatInfo, timingCache);
						const int32 slamDirection = Sign(point.v.vf - point.v.v);
						judgmentArray.emplace(y + ry, LaserSlamJudgment{ slamTimeSec, slamDirection });
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
				const auto& [sectionPulse, section] = *itr;
				if (!section.v.empty())
				{
					const auto& [relPulse, point] = *section.v.begin();
					return std::pair<double, bool>{ point.v.v, section.wide() };
				}
			}

			return none;
		}
	}

	LaserSlamJudgment::LaserSlamJudgment(double sec, int32 direction)
		: m_sec(sec)
		, m_direction(direction)
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

	JudgmentResult LaserSlamJudgment::result() const
	{
		return m_result;
	}

	void LaserSlamJudgment::setResult(JudgmentResult result)
	{
		m_result = result;
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

	JudgmentResult LaserSlamJudgment::judgmentResult(double currentTimeSec, IsAutoPlayYN isAutoPlay) const
	{
		if (isAutoPlay)
		{
			if (currentTimeSec < m_sec - TimingWindow::LaserNote::kWindowSecSlam)
			{
				return JudgmentResult::kUnspecified;
			}
			else
			{
				// 直角音のタイミングが遅れないよう、オートプレイ時は判定可能になったタイミングでやや先行してCRITICAL判定にする
				// (直角音は別途LaserSlamSE側でノーツ通りのタイミングで再生する仕組みになっているので、先行判定して問題ない)
				return JudgmentResult::kCritical;
			}
		}
		else
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

		// 入力を蓄積
		m_inputAccumulator.addDeltaCursorX(deltaCursorX);

		const int32 noteDirection = kson::ValueItrAt(m_laserLineDirectionMap, currentPulse)->second;
		const double noteCursorX = laneStatusRef.noteCursorX.value();
		const double cursorX = laneStatusRef.cursorX.value();
		double nextCursorX = cursorX;

		// 通常のカーソル移動(毎フレーム適用)
		const int32 direction = Sign(deltaCursorX);
		if (direction != 0)
		{
			if (Abs(cursorX - noteCursorX) < kLaserAutoFitMaxDeltaCursorX)
			{
				if (direction == noteDirection)
				{
					// ノーツと同方向の入力をしている場合は理想位置に吸い付かせる
					nextCursorX = noteCursorX;
					m_lastCorrectMovementSec = currentTimeSec;
				}
				else
				{
					// ノーツと逆方向の入力をしている場合はカーソルを動かさない
					nextCursorX = cursorX;
				}
			}
			else
			{
				// 移動後のカーソル位置(追い越し判定考慮前)
				const double movedCursorX = cursorX + deltaCursorX;

				// 理想位置に近づく方向に移動している場合のみ追い越し判定
				const bool isMovingTowardIdeal = (noteCursorX - cursorX) * direction > 0;
				if (isMovingTowardIdeal && Min(cursorX, movedCursorX) <= noteCursorX && noteCursorX <= Max(cursorX, movedCursorX))
				{
					nextCursorX = noteCursorX;
					m_lastCorrectMovementSec = currentTimeSec;
				}
				else
				{
					nextCursorX = movedCursorX;
				}
			}
		}

		// 1/60秒ごとにカーソル移動量を増幅して吸着させる
		if (m_inputAccumulator.shouldApplyAmplification(currentTimeSec))
		{
			const double accumulatedDeltaCursorX = m_inputAccumulator.getAccumulatedDeltaCursorX();
			const int32 amplifiedDirection = Sign(accumulatedDeltaCursorX);

			if (amplifiedDirection != 0)
			{
				// 増幅移動量での移動後のカーソル位置(追い越し判定考慮前)
				const double amplifiedCursorX = cursorX + accumulatedDeltaCursorX * kLaserCursorInputOvershootScale;

				// 理想位置に近づく方向に移動している場合のみ追い越し判定
				const bool isMovingTowardIdeal = (noteCursorX - cursorX) * amplifiedDirection > 0;
				if (isMovingTowardIdeal && Min(cursorX, amplifiedCursorX) - kLaserAutoFitMaxDeltaCursorX <= noteCursorX && noteCursorX <= Max(cursorX, amplifiedCursorX) + kLaserAutoFitMaxDeltaCursorX)
				{
					nextCursorX = noteCursorX;
					m_lastCorrectMovementSec = currentTimeSec;
				}
			}

			m_inputAccumulator.resetAccumulation(currentTimeSec);
		}

		laneStatusRef.cursorX = Clamp(nextCursorX, 0.0, 1.0);
	}

	void LaserLaneJudgment::processSlamJudgment(const kson::ByPulse<kson::LaserSection>& lane, double deltaCursorX, double currentTimeSec, LaserLaneStatus& laneStatusRef, JudgmentHandler& judgmentHandlerRef, IsAutoPlayYN isAutoPlay)
	{
		// 直角LASERはまだカーソルが出ていなくても先行判定するので、カーソルの存在チェックはしない

		// 既に判定済みの直角LASERをスキップしてカーソルを進める
		while (m_slamJudgmentArrayCursor != m_slamJudgmentArray.end())
		{
			const auto& [_, judgmentRef] = *m_slamJudgmentArrayCursor;
			if (judgmentRef.result() == JudgmentResult::kUnspecified)
			{
				break;
			}
			++m_slamJudgmentArrayCursor;
		}

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
		const JudgmentResult judgmentResult = laserSlamJudgmentRef.judgmentResult(currentTimeSec, isAutoPlay);
		if (judgmentResult != JudgmentResult::kUnspecified)
		{
			// 判定結果を記録
			laserSlamJudgmentRef.setResult(judgmentResult);

			judgmentHandlerRef.onLaserSlamJudged(judgmentResult, laserSlamPulse, m_prevTimeSecForDraw, m_prevPulse, laserSlamJudgmentRef.direction());

			if (judgmentResult == JudgmentResult::kCritical)
			{
				// 判定した時間を記録(補正および効果音再生に使用)
				laneStatusRef.lastLaserSlamJudgedTimeSec = Max(currentTimeSec, laserSlamJudgmentRef.sec());
				laneStatusRef.lastJudgedLaserSlamPulse = laserSlamPulse;

				// アニメーション
				const auto sectionItr = kson::GraphSectionAt(lane, laserSlamPulse);
				assert(sectionItr != lane.end() && "Cannot find laser section of slam note");
				const auto& [y, section] = *sectionItr;
				const auto& point = section.v.at(laserSlamPulse - y);
				laneStatusRef.rippleAnim.push({
					.startTimeSec = Max(laserSlamJudgmentRef.sec(), m_prevTimeSecForDraw),
					.wide = section.wide(),
					.x = point.v.vf,
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

				// 未判定の直角LASERが見つかったら抜ける
				const auto& [_, nextLaserSlamJudgmentRef] = *m_slamJudgmentArrayCursor;
				if (nextLaserSlamJudgmentRef.result() == JudgmentResult::kUnspecified)
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

	void LaserLaneJudgment::processAutoCursorMovementForAutoPlay(LaserLaneStatus& laneStatusRef)
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

		// カーソルを理想位置へ吸い付かせる
		laneStatusRef.cursorX = laneStatusRef.noteCursorX;
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

	void LaserLaneJudgment::processLineJudgment(const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse, LaserLaneStatus& laneStatusRef, JudgmentHandler& judgmentHandlerRef)
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
			judgmentHandlerRef.onLaserLineJudged(currentResult);
		}
	}

	void LaserLaneJudgment::processPassedLineJudgment(kson::Pulse currentPulse, JudgmentHandler& judgmentHandlerRef, IsAutoPlayYN isAutoPlay)
	{
		const JudgmentResult result = isAutoPlay ? JudgmentResult::kCritical : JudgmentResult::kError;
		for (auto itr = m_passedLineJudgmentCursor; itr != m_lineJudgmentArray.end(); ++itr)
		{
			auto& [y, judgment] = *itr;
			if (y + judgment.length < currentPulse)
			{
				// 通過済みの判定を処理
				if (judgment.result == JudgmentResult::kUnspecified)
				{
					judgment.result = result;
					judgmentHandlerRef.onLaserLineJudged(result);
				}

				m_passedLineJudgmentCursor = std::next(itr);
			}
		}
	}

	void LaserLaneJudgment::processPassedSlamJudgment(const kson::ByPulse<kson::LaserSection>& lane, double currentTimeSec, LaserLaneStatus& laneStatusRef, JudgmentHandler& judgmentHandlerRef, IsAutoPlayYN isAutoPlay)
	{
		using namespace TimingWindow;

		const JudgmentResult result = isAutoPlay ? JudgmentResult::kCritical : JudgmentResult::kError;
		const double thresholdSec = isAutoPlay ? 0.0 : LaserNote::kWindowSecSlam;

		for (auto itr = m_passedSlamJudgmentCursor; itr != m_slamJudgmentArray.end(); ++itr)
		{
			auto& [laserSlamPulse, laserSlamJudgmentRef] = *itr;

			// 既に判定済みの場合はスキップ
			if (laserSlamJudgmentRef.result() != JudgmentResult::kUnspecified)
			{
				m_passedSlamJudgmentCursor = std::next(itr);
				continue;
			}

			const double passSec = laserSlamJudgmentRef.sec() + thresholdSec;
			if (currentTimeSec < passSec)
			{
				// まだ通過していない判定に到達したのでループを抜ける
				break;
			}

			// 通過済みの直角LASER判定
			laserSlamJudgmentRef.setResult(result);
			judgmentHandlerRef.onLaserSlamJudged(result, laserSlamPulse, m_prevTimeSecForDraw, m_prevPulse, laserSlamJudgmentRef.direction());

			if (result == JudgmentResult::kCritical)
			{
				// 判定した時間を記録(補正および効果音再生に使用)
				laneStatusRef.lastLaserSlamJudgedTimeSec = Max(currentTimeSec, laserSlamJudgmentRef.sec());
				laneStatusRef.lastJudgedLaserSlamPulse = laserSlamPulse;

				// アニメーション
				const auto sectionItr = kson::GraphSectionAt(lane, laserSlamPulse);
				if (sectionItr != lane.end())
				{
					const auto& [y, section] = *sectionItr;
					const auto& point = section.v.at(laserSlamPulse - y);
					laneStatusRef.rippleAnim.push({
						.startTimeSec = Max(laserSlamJudgmentRef.sec(), m_prevTimeSecForDraw),
						.wide = section.wide(),
						.x = point.v.vf,
					});
				}
			}
			else
			{
				// 直角LASERがERROR判定になった場合は補正を切る
				m_lastCorrectMovementSec = kPastTimeSec;
			}

			m_passedSlamJudgmentCursor = std::next(itr);
		}
	}

	LaserLaneJudgment::LaserLaneJudgment(JudgmentPlayMode judgmentPlayMode, int32 laneIdx, const kson::ByPulse<kson::LaserSection>& lane, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache)
		: m_judgmentPlayMode(judgmentPlayMode)
		, m_laneIdx(laneIdx)
		, m_laserLineDirectionMap(CreateLaserLineDirectionMap(lane))
		, m_laserLineDirectionMapForRippleEffect(CreateLaserLineDirectionMapForRippleEffect(m_laserLineDirectionMap, lane))
		, m_laserLineDirectionChangeSecArray(CreateLaserLineDirectionChangeSecArray(lane, beatInfo, timingCache))
		, m_laserLineDirectionChangeSecArrayCursor(m_laserLineDirectionChangeSecArray.begin())
		, m_lineJudgmentArray(CreateLineJudgmentResultArray(lane, beatInfo, judgmentPlayMode))
		, m_passedLineJudgmentCursor(m_lineJudgmentArray.begin())
		, m_slamJudgmentArray(CreateSlamJudgmentArray(lane, beatInfo, timingCache, judgmentPlayMode))
		, m_slamJudgmentArrayCursor(m_slamJudgmentArray.begin())
		, m_passedSlamJudgmentCursor(m_slamJudgmentArray.begin())
	{
	}

	void LaserLaneJudgment::update(const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse, kson::Pulse currentPulseForDraw, double currentTimeSec, double currentTimeSecForDraw, LaserLaneStatus& laneStatusRef, JudgmentHandler& judgmentHandlerRef)
	{
		laneStatusRef.noteCursorX = kson::GraphSectionValueAt(lane, currentPulse);

		// 判定調整がある場合も、見かけ上は判定調整前のカーソル位置に描画
		const auto noteVisualCursorXOpt = kson::GraphSectionValueAt(lane, currentPulseForDraw);
		laneStatusRef.noteVisualCursorX = noteVisualCursorXOpt.has_value() ? noteVisualCursorXOpt.value() : laneStatusRef.noteCursorX;

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

		// 描画用のセクションパルスを計算
		Optional<kson::Pulse> currentLaserSectionPulseForDraw = none;
		if (noteVisualCursorXOpt.has_value())
		{
			const auto& [yDraw, laserSectionDraw] = *kson::ValueItrAt(lane, currentPulseForDraw);
			currentLaserSectionPulseForDraw = yDraw;
		}
		const bool hasSectionChanged = laneStatusRef.currentLaserSectionPulse != m_prevCurrentLaserSectionPulse;
		const bool hasSectionChangedForDraw = currentLaserSectionPulseForDraw != m_prevCurrentLaserSectionPulseForDraw;
		if (hasSectionChangedForDraw)
		{
			// LASERノーツ終点後のアニメーション
			if (m_prevCurrentLaserSectionPulseForDraw.has_value() && !currentLaserSectionPulseForDraw.has_value())
			{
				const auto& section = lane.at(m_prevCurrentLaserSectionPulseForDraw.value());
				assert(!section.v.empty() && "Laser section must not be empty");
				const auto& [lastPointRy, lastPoint] = *section.v.rbegin();
				const bool isLastPointSlam = !MathUtils::AlmostEquals(lastPoint.v.v, lastPoint.v.vf);
				if (!isLastPointSlam)
				{
					laneStatusRef.rippleAnim.push({
						.startTimeSec = m_prevTimeSecForDraw,
						.wide = section.wide(),
						.x = lastPoint.v.vf,
					});
				}
			}
		}
		if (hasSectionChanged)
		{
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
				laneStatusRef.cursorX = laserSection.v.begin()->second.v.v;
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

		if (m_judgmentPlayMode == JudgmentPlayMode::kOn)
		{
			// 入力からカーソルの移動量を取得
			const double deltaCursorX = KeyConfig::LaserDeltaCursorX(m_laneIdx, Scene::DeltaTime());
			processCursorMovement(deltaCursorX, currentPulse, currentTimeSec, laneStatusRef);
			processSlamJudgment(lane, deltaCursorX, currentTimeSec, laneStatusRef, judgmentHandlerRef, IsAutoPlayYN::No);

			// 直角LASER判定直後のカーソル自動移動
			processAutoCursorMovementBySlamJudgment(currentTimeSec, laneStatusRef);

			// 折り返し地点でのカーソル自動移動
			processAutoCursorMovementByLineDirectionChange(currentTimeSec, laneStatusRef);

			// 正解判定後のカーソル自動移動
			processAutoCursorMovementAfterCorrectMovement(currentTimeSec, laneStatusRef);

			// カーソル位置をもとにLASERの判定を決定
			processLineJudgment(lane, currentPulse, laneStatusRef, judgmentHandlerRef);

			// 通過済みのLASER判定をERRORにする
			processPassedLineJudgment(currentPulse, judgmentHandlerRef, IsAutoPlayYN::No);

			// 通過済みの直角LASER判定をERRORにする
			processPassedSlamJudgment(lane, currentTimeSec, laneStatusRef, judgmentHandlerRef, IsAutoPlayYN::No);
		}
		else if (m_judgmentPlayMode == JudgmentPlayMode::kAuto)
		{
			// オートプレイ中のカーソル自動移動
			processAutoCursorMovementForAutoPlay(laneStatusRef);

			// 直角判定
			processSlamJudgment(lane, 0.0, currentTimeSec, laneStatusRef, judgmentHandlerRef, IsAutoPlayYN::Yes);

			// 通過済みのLASER判定をCRITICALにする
			processPassedLineJudgment(currentPulse, judgmentHandlerRef, IsAutoPlayYN::Yes);

			// 通過済みの直角LASER判定をCRITICALにする
			processPassedSlamJudgment(lane, currentTimeSec, laneStatusRef, judgmentHandlerRef, IsAutoPlayYN::Yes);
		}
		else if (m_judgmentPlayMode == JudgmentPlayMode::kOff)
		{
			// Offモード時もカーソルを自動追従させる
			processAutoCursorMovementForAutoPlay(laneStatusRef);
		}

		// LASER折り返し波紋(描画用Critical範囲で判定、かつセクション内にいる場合のみ)
		if (m_prevIsCursorInCriticalJudgmentRangeForDraw && currentLaserSectionPulseForDraw.has_value())
		{
			const int32 direction = kson::ValueItrAt(m_laserLineDirectionMapForRippleEffect, currentPulseForDraw)->second;
			const int32 prevDirection = kson::ValueItrAt(m_laserLineDirectionMapForRippleEffect, m_prevPulseForDraw)->second;

			// 折り返し時のアニメーション
			if (direction != prevDirection && prevDirection != 0)
			{
				laneStatusRef.rippleAnim.push({
					.startTimeSec = currentTimeSecForDraw,
					.wide = laneStatusRef.cursorWide,
					.x = laneStatusRef.noteVisualCursorX.value(),
				});
			}
		}

		m_prevCurrentLaserSectionPulse = laneStatusRef.currentLaserSectionPulse;
		m_prevCurrentLaserSectionPulseForDraw = currentLaserSectionPulseForDraw;
		m_prevIsCursorInAutoFitRange =
			laneStatusRef.cursorX.has_value() &&
			laneStatusRef.noteCursorX.has_value() &&
			IsLaserCursorInAutoFitRange(laneStatusRef.cursorX.value(), laneStatusRef.noteCursorX.value());
		m_prevIsCursorInCriticalJudgmentRange = laneStatusRef.isCursorInCriticalJudgmentRange();
		m_prevIsCursorInCriticalJudgmentRangeForDraw =
			laneStatusRef.cursorX.has_value() &&
			laneStatusRef.noteVisualCursorX.has_value() &&
			IsLaserCursorInCriticalJudgmentRange(laneStatusRef.cursorX.value(), laneStatusRef.noteVisualCursorX.value());
		m_prevPulse = currentPulse;
		m_prevTimeSec = currentTimeSec;
		m_prevPulseForDraw = currentPulseForDraw;
		m_prevTimeSecForDraw = currentTimeSecForDraw;
	}

	void LaserLaneJudgment::lockForExit(JudgmentHandler& judgmentHandlerRef)
	{
		m_isLockedForExit = true;

		// ラインノーツの未判定をERRORにする
		for (auto& [pulse, lineJudgment] : m_lineJudgmentArray)
		{
			if (lineJudgment.result == JudgmentResult::kUnspecified)
			{
				lineJudgment.result = JudgmentResult::kError;
				judgmentHandlerRef.onLaserLineJudged(JudgmentResult::kError);
			}
		}

		// 直角LASERの未判定をERRORにする
		for (auto it = m_slamJudgmentArrayCursor; it != m_slamJudgmentArray.end(); ++it)
		{
			auto& [pulse, slamJudgment] = *it;
			if (slamJudgment.result() == JudgmentResult::kUnspecified)
			{
				slamJudgment.setResult(JudgmentResult::kError);
				judgmentHandlerRef.onLaserSlamJudged(JudgmentResult::kError, pulse, m_prevTimeSec, m_prevPulse, slamJudgment.direction());
			}
		}
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
