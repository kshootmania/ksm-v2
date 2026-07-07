#include "TurnUtil.hpp"
#include "kson/Util/TimingUtils.hpp"

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

		// 正規譜面のBT・FXノーツの列挙用情報
		struct SRandomNoteRef
		{
			kson::Pulse y = kson::Pulse{ 0 };

			// レーン番号(0〜3がBT-A〜D、4〜5がFX-L/R)
			std::size_t laneIdx = 0U;

			kson::Interval interval;
		};

		// レーン番号(0〜5)からノーツ一覧への参照を取得
		const kson::ByPulse<kson::Interval>& NoteLaneAt(const kson::ChartData& chartData, std::size_t laneIdx)
		{
			if (laneIdx < kson::kNumBTLanesSZ)
			{
				return chartData.note.bt[laneIdx];
			}
			return chartData.note.fx[laneIdx - kson::kNumBTLanesSZ];
		}

		// 正規譜面のBT・FX全ノーツをpulse昇順(同pulse内はレーン昇順)で列挙
		Array<SRandomNoteRef> EnumerateSRandomNotes(const kson::ChartData& chartData)
		{
			Array<SRandomNoteRef> notes;
			for (std::size_t laneIdx = 0U; laneIdx < kNumSRandomLanesSZ; ++laneIdx)
			{
				for (const auto& [y, interval] : NoteLaneAt(chartData, laneIdx))
				{
					notes.push_back(SRandomNoteRef
					{
						.y = y,
						.laneIdx = laneIdx,
						.interval = interval,
					});
				}
			}
			notes.sort_by([](const SRandomNoteRef& a, const SRandomNoteRef& b)
			{
				if (a.y != b.y)
				{
					return a.y < b.y;
				}
				return a.laneIdx < b.laneIdx;
			});
			return notes;
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

	Array<std::size_t> MakeSRandomNoteLanes(const kson::ChartData& chartData, double playbackSpeed)
	{
		const Array<SRandomNoteRef> notes = EnumerateSRandomNotes(chartData);
		const kson::TimingCache timingCache = kson::CreateTimingCache(chartData.beat);

		// 縦連を避ける間隔のしきい値(秒)
		const double jackAvoidanceThresholdSec = kSRandomJackAvoidanceIntervalSec * playbackSpeed;

		// 各レーンがロングノーツで埋まっている終了pulse
		std::array<kson::Pulse, kNumSRandomLanesSZ> occupiedUntil;
		occupiedUntil.fill(kPastPulse);

		// 各レーンに最後に配置したノーツの終了時刻(秒)
		std::array<double, kNumSRandomLanesSZ> lastNoteEndSec;
		lastNoteEndSec.fill(kPastTimeSec);

		Array<std::size_t> noteLanes;
		noteLanes.reserve(notes.size());

		// 同時押しグループ(開始pulseが同じノーツ)単位で空きレーンへランダムに割り当て
		for (std::size_t groupBegin = 0U; groupBegin < notes.size();)
		{
			std::size_t groupEnd = groupBegin;
			while (groupEnd < notes.size() && notes[groupEnd].y == notes[groupBegin].y)
			{
				++groupEnd;
			}

			const kson::Pulse y = notes[groupBegin].y;
			const double ySec = kson::PulseToSec(y, chartData.beat, timingCache);
			Array<std::size_t> freeLanes;
			for (std::size_t laneIdx = 0U; laneIdx < kNumSRandomLanesSZ; ++laneIdx)
			{
				if (occupiedUntil[laneIdx] <= y)
				{
					freeLanes.push_back(laneIdx);
				}
			}
			freeLanes.shuffle();

			for (std::size_t i = groupBegin; i < groupEnd; ++i)
			{
				// 秒数しきい値で縦連を避ける
				std::size_t chosenIdxInFreeLanes = freeLanes.size();
				for (std::size_t j = 0U; j < freeLanes.size(); ++j)
				{
					if (ySec - lastNoteEndSec[freeLanes[j]] >= jackAvoidanceThresholdSec)
					{
						chosenIdxInFreeLanes = j;
						break;
					}
				}
				if (chosenIdxInFreeLanes == freeLanes.size() && !freeLanes.isEmpty())
				{
					// 縦連を避けられない場合は空きレーンの先頭を使用
					chosenIdxInFreeLanes = 0U;
				}

				// 正規譜面が正常であれば空きレーン数は必ず足りるはず(同時に配置されるノーツ数はレーンに依存しないため)
				// 万が一足りなかった場合は元のレーンを維持する
				std::size_t newLaneIdx;
				if (chosenIdxInFreeLanes < freeLanes.size())
				{
					newLaneIdx = freeLanes[chosenIdxInFreeLanes];
					freeLanes.remove_at(chosenIdxInFreeLanes);
				}
				else
				{
					newLaneIdx = notes[i].laneIdx;
				}
				noteLanes.push_back(newLaneIdx);

				if (notes[i].interval.length > kson::RelPulse{ 0 })
				{
					occupiedUntil[newLaneIdx] = y + notes[i].interval.length;
					lastNoteEndSec[newLaneIdx] = kson::PulseToSec(y + notes[i].interval.length, chartData.beat, timingCache);
				}
				else
				{
					lastNoteEndSec[newLaneIdx] = ySec;
				}
			}

			groupBegin = groupEnd;
		}

		return noteLanes;
	}

	bool ApplySRandomNoteLanes(kson::ChartData* pChartData, const Array<std::size_t>& noteLanes)
	{
		const Array<SRandomNoteRef> notes = EnumerateSRandomNotes(*pChartData);
		if (notes.size() != noteLanes.size())
		{
			return false;
		}

		kson::BTLane<kson::Interval> newBTLanes;
		kson::FXLane<kson::Interval> newFXLanes;
		for (std::size_t i = 0U; i < notes.size(); ++i)
		{
			const std::size_t newLaneIdx = noteLanes[i];
			if (newLaneIdx >= kNumSRandomLanesSZ)
			{
				return false;
			}
			auto& newLane = newLaneIdx < kson::kNumBTLanesSZ
				? newBTLanes[newLaneIdx]
				: newFXLanes[newLaneIdx - kson::kNumBTLanesSZ];
			const auto [itr, inserted] = newLane.emplace(notes[i].y, notes[i].interval);
			if (!inserted)
			{
				// 同一レーンの同一pulseに複数のノーツが配置されるデータは不正
				return false;
			}
		}

		pChartData->note.bt = std::move(newBTLanes);
		pChartData->note.fx = std::move(newFXLanes);
		return true;
	}

	std::array<kson::ByPulse<std::size_t>, kson::kNumFXLanesSZ> MakeFXNoteDestinations(const kson::ChartData& chartData, const Array<std::size_t>& noteLanes)
	{
		const Array<SRandomNoteRef> notes = EnumerateSRandomNotes(chartData);
		assert(notes.size() == noteLanes.size());

		std::array<kson::ByPulse<std::size_t>, kson::kNumFXLanesSZ> destinations;
		const std::size_t count = Min(notes.size(), noteLanes.size());
		for (std::size_t i = 0U; i < count; ++i)
		{
			if (notes[i].laneIdx >= kson::kNumBTLanesSZ)
			{
				destinations[notes[i].laneIdx - kson::kNumBTLanesSZ].emplace(notes[i].y, noteLanes[i]);
			}
		}
		return destinations;
	}
}
