#include "laser_lane_judgment.hpp"
#include "kson/util/graph_utils.hpp"

namespace MusicGame::Judgment
{
	namespace
	{
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

		const int32 direction = button == m_keyConfigButtonL ? -1 : 1;
		laneStatusRef.cursorX = laneStatusRef.cursorX.value() + kLaserKeyboardCursorXPerSec * Scene::DeltaTime() * direction;

		Print << U"direction:" << direction << U", value:" << laneStatusRef.cursorX << U"," << laneStatusRef.noteCursorX;
	}

	LaserLaneJudgment::LaserLaneJudgment(KeyConfig::Button keyConfigButtonL, KeyConfig::Button keyConfigButtonR, const kson::ByPulse<kson::LaserSection>& lane, const kson::BeatInfo& beatInfo, const kson::TimingCache& timingCache)
		: m_keyConfigButtonL(keyConfigButtonL)
		, m_keyConfigButtonR(keyConfigButtonR)
		, m_lineJudgmentArray(CreateLineJudgmentArray(lane, beatInfo))
		, m_slamJudgmentArray(CreateSlamJudgmentArray(lane))
		, m_scoreValueMax(static_cast<int32>(m_lineJudgmentArray.size() + m_slamJudgmentArray.size())* kScoreValueCritical)
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
