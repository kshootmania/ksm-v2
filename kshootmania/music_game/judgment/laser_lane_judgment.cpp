#include "laser_lane_judgment.hpp"
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
					prevValue = v.v;
				}
				directionMap.insert_or_assign(prevPulse, 0);
			}

			return directionMap;
		}

		kson::ByPulse<JudgmentResult> CreateLineJudgmentArray(const kson::ByPulse<kson::LaserSection>& lane, const kson::BeatInfo& beatInfo)
		{
			kson::ByPulse<JudgmentResult> judgmentArray;

			for (const auto& [y, sec] : lane)
			{
				if (sec.v.empty())
				{
					continue;
				}
				
				const kson::RelPulse lastRy = sec.v.rbegin()->first;
				if (lastRy > 0)
				{
					// BPMをもとにLASERノーツのコンボ数を半減させるかを決める
					// (セクション途中でのBPM変更は特に加味しない)
					const bool halvesCombo = kson::TempoAt(y, beatInfo) >= kHalveComboBPMThreshold;
					const kson::RelPulse minPulseInterval = halvesCombo ? (kson::kResolution4 * 3 / 8) : (kson::kResolution4 * 3 / 16);
					const kson::RelPulse pulseInterval = halvesCombo ? (kson::kResolution4 / 8) : (kson::kResolution4 / 16);

					if (lastRy <= minPulseInterval)
					{
						judgmentArray.emplace(y, JudgmentResult::kUnspecified);
					}
					else
					{
						const kson::Pulse start = ((y + pulseInterval - 1) / pulseInterval + 1) * pulseInterval;
						const kson::Pulse end = y + lastRy - pulseInterval;

						for (kson::Pulse pulse = start; pulse < end; pulse += pulseInterval)
						{
							judgmentArray.emplace(pulse, JudgmentResult::kUnspecified);
						}
					}
				}
			}

			return judgmentArray;
		}

		kson::ByPulse<JudgmentResult> CreateSlamJudgmentArray(const kson::ByPulse<kson::LaserSection>& lane)
		{
			kson::ByPulse<JudgmentResult> judgmentArray;

			for (const auto& [y, sec] : lane)
			{
				for (const auto& [ry, point] : sec.v)
				{
					const bool isSlam = !kson::AlmostEquals(point.v, point.vf);
					if (isSlam)
					{
						judgmentArray.emplace(y + ry, JudgmentResult::kUnspecified);
					}
				}
			}

			return judgmentArray;
		}

		Optional<double> GetPregeneratedCursorValue(const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse)
		{
			// レーザーの値が存在すればカーソルが出ている
			/*const auto graphSectionValue = kson::GraphSectionValueAt(lane, currentPulse);
			if (graphSectionValue.has_value())
			{
				return graphSectionValue.value();
			}*/

			// 直近1小節以内にレーザーセクションの始点が存在すればカーソルが出ている
			const auto itr = kson::FirstInRange(lane, currentPulse, currentPulse + kson::kResolution + kson::Pulse{ 1 });
			if (itr != lane.end())
			{
				const auto& [_, sec] = *itr;
				if (!sec.v.empty())
				{
					const auto& [_, v] = *sec.v.begin();
					return v.v;
				}
			}

			return none;
		}
	}

	void LaserLaneJudgment::processKeyPressed(KeyConfig::Button button, const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse, LaserLaneStatus& laneStatusRef)
	{
		assert(button == m_keyConfigButtonL || button == m_keyConfigButtonR);

		if (!laneStatusRef.cursorX.has_value())
		{
			// カーソルが出ていない場合は何もしない
			return;
		}

		if (!laneStatusRef.noteCursorX.has_value())
		{
			// 事前生成されたカーソルは動かさない
			return;
		}

		const int32 direction = button == m_keyConfigButtonL ? -1 : 1;
		const int32 noteDirection = kson::ValueItrAt(m_laserLineDirectionMap, currentPulse)->second;
		const double noteCursorX = laneStatusRef.noteCursorX.value();
		const double cursorX = laneStatusRef.cursorX.value();
		const double deltaCursorX = kLaserKeyboardCursorXPerSec * Scene::DeltaTime() * direction;
		double nextCursorX;
		if (direction == noteDirection || noteDirection == 0)
		{
			// LASERノーツと同方向にカーソル移動している、または、LASERノーツが横移動なしの場合
			const double overshootCursorX = cursorX + deltaCursorX * 5;
			if (Min(cursorX, overshootCursorX) <= noteCursorX && noteCursorX <= Max(cursorX, overshootCursorX))
			{
				// 理想カーソル位置が移動量を増幅して動かした場合の範囲に入っていれば、カーソルを理想カーソル位置へ吸い付かせる
				nextCursorX = noteCursorX;
			}
			else
			{
				// 足りなければ、カーソルを単純に動かす
				nextCursorX = cursorX + deltaCursorX;
			}
		}
		else
		{
			// LASERノーツと逆方向にカーソル移動している場合、カーソルを単純に動かす
			nextCursorX = cursorX + deltaCursorX;
		}
		laneStatusRef.cursorX = Clamp(nextCursorX, 0.0, 1.0);

		Print << U"direction:" << direction << U", noteDirection:" << noteDirection << U", value:" << laneStatusRef.cursorX << U"," << laneStatusRef.noteCursorX;
	}

	LaserLaneJudgment::LaserLaneJudgment(KeyConfig::Button keyConfigButtonL, KeyConfig::Button keyConfigButtonR, const kson::ByPulse<kson::LaserSection>& lane, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache)
		: m_keyConfigButtonL(keyConfigButtonL)
		, m_keyConfigButtonR(keyConfigButtonR)
		, m_laserLineDirectionMap(CreateLaserLineDirectionMap(lane))
		, m_lineJudgmentArray(CreateLineJudgmentArray(lane, beatInfo))
		, m_slamJudgmentArray(CreateSlamJudgmentArray(lane))
		, m_scoreValueMax(static_cast<int32>(m_lineJudgmentArray.size() + m_slamJudgmentArray.size()) * kScoreValueCritical)
	{
	}

	void LaserLaneJudgment::update(const kson::ByPulse<kson::LaserSection>& lane, kson::Pulse currentPulse, double currentSec, LaserLaneStatus& laneStatusRef)
	{
		laneStatusRef.noteCursorX = kson::GraphSectionValueAt(lane, currentPulse);
		laneStatusRef.noteVisualCursorX = laneStatusRef.noteCursorX; // TODO: タイミング調整に合わせてずらして取得

		const auto pregeneratedCursorValue = GetPregeneratedCursorValue(lane, currentPulse);
		if (laneStatusRef.cursorX.has_value())
		{
			// カーソルが消滅
			if (!laneStatusRef.noteCursorX.has_value() && !pregeneratedCursorValue.has_value())
			{
				laneStatusRef.cursorX = none;
			}
		}
		else
		{
			// カーソルが出現
			if (laneStatusRef.noteCursorX.has_value())
			{
				// 既にレーザーに突入している場合はその値に合わせる
				// (1小節前に事前生成されるので、BPMが非常に速い場合以外は通らない)
				laneStatusRef.cursorX = laneStatusRef.noteCursorX;
			}
			else if (pregeneratedCursorValue.has_value())
			{
				// 次のレーザーセクションの始点の値に合わせる
				laneStatusRef.cursorX = pregeneratedCursorValue;
			}
		}

		// 現在判定対象になっているLASERセクションの始点Pulse値を取得
		if (laneStatusRef.noteCursorX.has_value())
		{
			laneStatusRef.currentLaserSectionPulse = kson::ValueItrAt(lane, currentPulse)->first;
		}
		else
		{
			laneStatusRef.currentLaserSectionPulse = none;
		}

		// キー押下中の判定処理
		const Optional<KeyConfig::Button> lastPressedButton = KeyConfig::LastPressed(m_keyConfigButtonL, m_keyConfigButtonR);
		if (lastPressedButton.has_value())
		{
			processKeyPressed(lastPressedButton.value(), lane, currentPulse, laneStatusRef);
		}
	}

	int32 LaserLaneJudgment::scoreValue() const
	{
		return m_scoreValue;
	}

	int32 LaserLaneJudgment::scoreValueMax() const
	{
		return m_scoreValueMax;
	}
}
