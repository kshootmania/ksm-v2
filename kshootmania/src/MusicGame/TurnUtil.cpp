#include "TurnUtil.hpp"

namespace MusicGame
{
	namespace
	{
		// BTレーンを入れ替え
		void ApplyBTLaneTurn(kson::BTLane<kson::Interval>& btLanes, const std::array<std::size_t, kson::kNumBTLanesSZ>& btLaneTable)
		{
			// 元のレーンデータをコピー
			const auto originalBT = btLanes;

			// テーブルに従ってレーンを入れ替え
			for (std::size_t i = 0U; i < kson::kNumBTLanesSZ; ++i)
			{
				btLanes[i] = originalBT[btLaneTable[i]];
			}
		}

		// FXレーンを入れ替え
		void ApplyFXLaneTurn(kson::FXLane<kson::Interval>& fxLanes, bool invert)
		{
			if (invert)
			{
				std::swap(fxLanes[0], fxLanes[1]);
			}
		}

		template <typename T>
		void ApplyFXLaneTurnToDict(kson::Dict<kson::FXLane<T>>& dict, bool invert)
		{
			if (!invert)
			{
				return;
			}

			for (auto& [name, fxLanes] : dict)
			{
				std::swap(fxLanes[0], fxLanes[1]);
			}
		}

		// LASERレーンを入れ替え
		void ApplyLaserLaneTurn(kson::LaserLane<kson::LaserSection>& laserLanes, bool invert)
		{
			if (invert)
			{
				std::swap(laserLanes[0], laserLanes[1]);
			}
		}

		// LASER値左右反転
		void InvertLaserValues(kson::LaserLane<kson::LaserSection>& laserLanes)
		{
			for (auto& laserSections : laserLanes)
			{
				for (auto& [pulse, laserSection] : laserSections)
				{
					for (auto& [relPulse, value] : laserSection.v)
					{
						value.v.v = 1.0 - value.v.v;
						value.v.vf = 1.0 - value.v.vf;
					}
				}
			}
		}

		// カメラエフェクトの左右反転(手動傾き/回転/半回転/ばね/zoom_side)
		void InvertCameraEffects(kson::CameraInfo& camera)
		{
			// 手動傾き左右反転
			for (auto& [pulse, tiltValue] : camera.tilt)
			{
				if (std::holds_alternative<kson::TiltGraphPoint>(tiltValue))
				{
					kson::TiltGraphPoint& point = std::get<kson::TiltGraphPoint>(tiltValue);
					point.v.v = -point.v.v;
					if (std::holds_alternative<double>(point.v.vf))
					{
						point.v.vf = -std::get<double>(point.v.vf);
					}
				}
			}

			// zoom_side左右反転
			for (auto& [pulse, value] : camera.cam.body.zoomSide)
			{
				value.v.v = -value.v.v;
				value.v.vf = -value.v.vf;
			}

			// rotation_deg左右反転
			for (auto& [pulse, value] : camera.cam.body.rotationDeg)
			{
				value.v.v = -value.v.v;
				value.v.vf = -value.v.vf;
			}

			// 回転エフェクト左右反転
			for (auto& [pulse, spin] : camera.cam.pattern.laser.slamEvent.spin)
			{
				spin.d = -spin.d;
			}

			// 半回転エフェクト左右反転
			for (auto& [pulse, halfSpin] : camera.cam.pattern.laser.slamEvent.halfSpin)
			{
				halfSpin.d = -halfSpin.d;
			}

			// ばねエフェクト左右反転
			for (auto& [pulse, swing] : camera.cam.pattern.laser.slamEvent.swing)
			{
				swing.d = -swing.d;
			}
		}
	}

	void ApplyTurnTable(kson::ChartData& chartData, const TurnTable& turnTable)
	{
		// BTレーン入れ替え
		ApplyBTLaneTurn(chartData.note.bt, turnTable.btLaneTable);

		// FXレーン入れ替え
		ApplyFXLaneTurn(chartData.note.fx, turnTable.invertFXLane);
		ApplyFXLaneTurnToDict(chartData.audio.audioEffect.fx.longEvent, turnTable.invertFXLane);
		ApplyFXLaneTurnToDict(chartData.audio.keySound.fx.chipEvent, turnTable.invertFXLane);

		// LASERレーン入れ替え
		ApplyLaserLaneTurn(chartData.note.laser, turnTable.invertLaserLane);

		// LASER値左右反転
		if (turnTable.invertLaserValue)
		{
			InvertLaserValues(chartData.note.laser);
		}

		// カメラエフェクト左右反転
		if (turnTable.invertLaserValue)
		{
			InvertCameraEffects(chartData.camera);
		}
	}
}
