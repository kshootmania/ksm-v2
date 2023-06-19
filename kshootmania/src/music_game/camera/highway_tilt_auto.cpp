#include "highway_tilt_auto.hpp"
#include "music_game/game_defines.hpp"
#include "kson/util/graph_utils.hpp"

namespace MusicGame::Camera
{
	namespace
	{
		constexpr double kSlowDownDiffThreshold = 0.1;
		constexpr double kZeroTiltFactorThreshold = 0.001;
		constexpr double kZeroTiltSlowDownFactor = 1.0 / 5;
		constexpr double kMinSpeed = 0.5;
		constexpr double kZeroAbsTiltFactorThresholdForKeep = 0.018;
		constexpr double kTiltScaleInterpolationSpeed = 4.0;

		/// @brief 現在の傾き目標値を取得する
		/// @param laserLanes LASERノーツのレーン
		/// @param currentPulse 現在のPulse値
		/// @return 現在の傾き目標値(-1.0 ～ 1.0)
		double GetTargetTiltFactor(const kson::LaserLane<kson::LaserSection>& laserLanes, const kson::Pulse currentPulse)
		{
			double tiltFactor = 0.0;
			for (std::size_t i = 0; i < laserLanes.size(); ++i)
			{
				const kson::ByPulse<kson::LaserSection>& lane = laserLanes[i];
				const bool isLeftLaser = i == 0;

				const auto currentNoteCursorX = kson::GraphSectionValueAt(lane, currentPulse);
				if (currentNoteCursorX.has_value())
				{
					// 現在LASERセクション内にいる場合はその値を使用
					const double value = currentNoteCursorX.value();
					tiltFactor += isLeftLaser ? value : -(1.0 - value);
				}
				else
				{
					// 現在LASERセクション内にいない場合も、直近0.5小節以内にレーザーセクションの始点が存在すれば事前に傾かせる
					const auto itr = kson::FirstInRange(lane, currentPulse, currentPulse + kson::kResolution4 / 2);
					if (itr != lane.end())
					{
						const auto& [_, section] = *itr;
						if (!section.v.empty())
						{
							const auto& [_, v] = *section.v.begin();
							tiltFactor += isLeftLaser ? v.v : -(1.0 - v.v);
						}
					}
				}
			}
			return tiltFactor;
		}

		/// @brief 傾き目標値に近づける速度を取得する
		/// @param targetTiltFactor 傾き目標値
		/// @param currentTiltFactor 現在の傾き
		/// @return 速度
		double Speed(double targetTiltFactor, double currentTiltFactor)
		{
			double speed = 4.5;

			// 目標の傾きに近くなった場合は減速させる
			const double diff = Abs(targetTiltFactor - currentTiltFactor);
			if (diff < kSlowDownDiffThreshold)
			{
				speed *= diff / kSlowDownDiffThreshold;
			}

			// 目標の傾きがゼロに近い場合は遅くする
			if (Abs(targetTiltFactor) < kZeroTiltFactorThreshold)
			{
				speed *= kZeroTiltSlowDownFactor;
			}

			// スピードが遅くなりすぎないようにする(目標値に収束させるため)
			speed = Max(speed, kMinSpeed);

			return speed;
		}

		/// @brief 現在の値を目標値に近づける
		/// @param current 現在の値
		/// @param target 目標値
		/// @param speed 速度
		/// @param deltaTime 1フレームの時間
		/// @return 目標値へ近づけた値
		double Interpolate(double current, double target, double speed, double deltaTime)
		{
			if (current < target)
			{
				return Min(current + deltaTime * speed, target);
			}
			else
			{
				return Max(current - deltaTime * speed, target);
			}
		}
	}

	void HighwayTiltAuto::updateTiltFactor(const kson::LaserLane<kson::LaserSection>& lanes, const kson::TiltInfo& tilt, kson::Pulse currentPulse)
	{
		const double prevSmoothedTiltFactor = m_smoothedTiltFactor;

		// 目標値に線形で近づける
		double targetTiltFactor = GetTargetTiltFactor(lanes, currentPulse);
		const double speed = Speed(targetTiltFactor, m_smoothedTiltFactor);
		m_smoothedTiltFactor = Interpolate(m_smoothedTiltFactor, targetTiltFactor, speed, Scene::DeltaTime());

		// 傾きキープ
		const bool keepEnabled = kson::ValueAtOrDefault(tilt.keep, currentPulse, false);
		if (keepEnabled)
		{
			// 前回フレームより傾きが小さいor逆方向に傾いている場合はキープ
			// HSP版: https://github.com/kshootmania/ksm-v1/blob/b26026420fa164310bf25f93c218bb83480faef8/src/scene/play/play_laser_note.hsp#L149
			if (Abs(targetTiltFactor) < Abs(m_prevTargetTiltFactor) || (Sign(targetTiltFactor) + Sign(m_prevTargetTiltFactor) == 0 && Abs(prevSmoothedTiltFactor) > kZeroAbsTiltFactorThresholdForKeep))
			{
				// キープされた場合は前回フレームの傾きで上書き
				m_smoothedTiltFactor = prevSmoothedTiltFactor;
				targetTiltFactor = m_prevTargetTiltFactor;
			}
		}

		m_prevTargetTiltFactor = targetTiltFactor;
	}

	void HighwayTiltAuto::updateTiltScale(const kson::LaserLane<kson::LaserSection>& lanes, const kson::TiltInfo& tilt, kson::Pulse currentPulse)
	{
		const double targetTiltScale = kson::ValueAtOrDefault(tilt.scale, currentPulse, 1.0);
		m_tiltScale = Interpolate(m_tiltScale, targetTiltScale, kTiltScaleInterpolationSpeed, Scene::DeltaTime());
	}

	void HighwayTiltAuto::update(const kson::LaserLane<kson::LaserSection>& lanes, const kson::TiltInfo& tilt, kson::Pulse currentPulse)
	{
		updateTiltFactor(lanes, tilt, currentPulse);
		updateTiltScale(lanes, tilt, currentPulse);
	}

	double HighwayTiltAuto::radians() const
	{
		return kTiltRadians * m_smoothedTiltFactor * m_tiltScale;
	}
}
