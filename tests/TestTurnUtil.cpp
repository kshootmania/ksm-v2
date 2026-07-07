#include <catch2/catch.hpp>
#include "MusicGame/TurnUtil.hpp"

TEST_CASE("ApplyTurnTable mirrors FX lane attached events", "[TurnUtil]")
{
	kson::ChartData chartData;

	chartData.note.fx[0].emplace(100, kson::Interval{ .length = 240 });
	chartData.note.fx[1].emplace(200, kson::Interval{ .length = 0 });

	kson::AudioEffectParams params;
	params.emplace("wave_length", "1/8");
	chartData.audio.audioEffect.fx.longEvent["retrig"][0].emplace(100, params);

	chartData.audio.keySound.fx.chipEvent["clap"][1].emplace(200, kson::KeySoundInvokeFX{ .vol = 0.5 });

	MusicGame::ApplyTurnTable(chartData, MusicGame::MakeTurnTable(TurnMode::kMirror));

	REQUIRE(chartData.note.fx[0].contains(200));
	REQUIRE(chartData.note.fx[1].contains(100));

	REQUIRE(chartData.audio.audioEffect.fx.longEvent["retrig"][0].empty());
	REQUIRE(chartData.audio.audioEffect.fx.longEvent["retrig"][1].contains(100));
	REQUIRE(chartData.audio.audioEffect.fx.longEvent["retrig"][1].at(100).at("wave_length") == "1/8");

	REQUIRE(chartData.audio.keySound.fx.chipEvent["clap"][0].contains(200));
	REQUIRE(chartData.audio.keySound.fx.chipEvent["clap"][0].at(200).vol == Approx(0.5));
	REQUIRE(chartData.audio.keySound.fx.chipEvent["clap"][1].empty());
}

namespace
{
	// レーン番号(0〜5)からノーツ一覧への参照を取得
	const kson::ByPulse<kson::Interval>& NoteLaneAt(const kson::ChartData& chartData, std::size_t laneIdx)
	{
		if (laneIdx < kson::kNumBTLanesSZ)
		{
			return chartData.note.bt[laneIdx];
		}
		return chartData.note.fx[laneIdx - kson::kNumBTLanesSZ];
	}

	// BT・FX全ノーツの(pulse, length)一覧をレーン横断で取得(配置に依存しない同一性の比較用)
	Array<std::pair<kson::Pulse, kson::RelPulse>> AllNotes(const kson::ChartData& chartData)
	{
		Array<std::pair<kson::Pulse, kson::RelPulse>> notes;
		for (std::size_t laneIdx = 0U; laneIdx < MusicGame::kNumSRandomLanesSZ; ++laneIdx)
		{
			for (const auto& [y, interval] : NoteLaneAt(chartData, laneIdx))
			{
				notes.emplace_back(y, interval.length);
			}
		}
		notes.sort();
		return notes;
	}

	// 各レーン内でノーツ同士が重ならないかを確認
	bool HasNoOverlap(const kson::ChartData& chartData)
	{
		for (std::size_t laneIdx = 0U; laneIdx < MusicGame::kNumSRandomLanesSZ; ++laneIdx)
		{
			kson::Pulse prevEndY = std::numeric_limits<kson::Pulse>::min();
			for (const auto& [y, interval] : NoteLaneAt(chartData, laneIdx))
			{
				if (y < prevEndY)
				{
					return false;
				}
				prevEndY = y + interval.length;
			}
		}
		return true;
	}
}

TEST_CASE("ApplySRandomNoteLanes applies assignment deterministically across BT/FX", "[TurnUtil]")
{
	kson::ChartData chartData;
	chartData.note.bt[0].emplace(0, kson::Interval{ .length = 0 });
	chartData.note.fx[0].emplace(0, kson::Interval{ .length = 0 });
	chartData.note.bt[2].emplace(100, kson::Interval{ .length = 50 });
	chartData.note.fx[1].emplace(200, kson::Interval{ .length = 60 });

	// エフェクト辞書は配置適用で変更されないことを確認するため設定しておく
	kson::AudioEffectParams params;
	params.emplace("wave_length", "1/8");
	chartData.audio.audioEffect.fx.longEvent["retrig"][1].emplace(200, params);

	// BT-AのチップをFX-Rへ、FX-LのチップをBT-Bへ、BT-CのロングをFX-Lへ、FX-RのロングをBT-Aへ配置
	const Array<std::size_t> noteLanes{ 5, 1, 4, 0 };

	REQUIRE(MusicGame::ApplySRandomNoteLanes(&chartData, noteLanes));

	// BTチップがFX-Rへ移動
	REQUIRE(chartData.note.fx[1].contains(0));
	// FXチップがBT-Bへ移動
	REQUIRE(chartData.note.bt[1].contains(0));
	// BTロングがFX-Lへ移動
	REQUIRE(chartData.note.fx[0].contains(100));
	REQUIRE(chartData.note.fx[0].at(100).length == 50);
	// FXロングがBT-Aへ移動
	REQUIRE(chartData.note.bt[0].contains(200));
	REQUIRE(chartData.note.bt[0].at(200).length == 60);

	// エフェクト辞書は元FXレーン基準のまま変更されない
	REQUIRE(chartData.audio.audioEffect.fx.longEvent["retrig"][1].contains(200));
}

TEST_CASE("ApplySRandomNoteLanes rejects invalid assignment", "[TurnUtil]")
{
	kson::ChartData chartData;
	chartData.note.bt[0].emplace(0, kson::Interval{ .length = 0 });
	chartData.note.fx[0].emplace(0, kson::Interval{ .length = 0 });

	// ノーツ数と配列長の不一致
	{
		kson::ChartData copied = chartData;
		REQUIRE(!MusicGame::ApplySRandomNoteLanes(&copied, Array<std::size_t>{ 0 }));
	}

	// 同一レーンの同一pulseへの重複配置
	{
		kson::ChartData copied = chartData;
		REQUIRE(!MusicGame::ApplySRandomNoteLanes(&copied, Array<std::size_t>{ 2, 2 }));
	}

	// レーン番号の範囲外
	{
		kson::ChartData copied = chartData;
		REQUIRE(!MusicGame::ApplySRandomNoteLanes(&copied, Array<std::size_t>{ 0, 6 }));
	}
}

TEST_CASE("MakeSRandomNoteLanes generates valid assignment", "[TurnUtil]")
{
	// ロングノーツと同時押しを含む譜面で複数回生成し、常に妥当な配置になることを確認
	kson::ChartData chartData;
	chartData.note.bt[0].emplace(0, kson::Interval{ .length = 480 }); // ロング(0〜480)
	chartData.note.bt[1].emplace(0, kson::Interval{ .length = 0 });
	chartData.note.fx[0].emplace(0, kson::Interval{ .length = 240 }); // FXロング(0〜240)
	chartData.note.bt[1].emplace(240, kson::Interval{ .length = 0 }); // ロング中のチップ
	chartData.note.bt[2].emplace(240, kson::Interval{ .length = 0 }); // ロング中のチップ
	chartData.note.fx[1].emplace(240, kson::Interval{ .length = 120 }); // ロング中のFXロング
	chartData.note.bt[0].emplace(480, kson::Interval{ .length = 0 }); // ロング終端と同時のチップ
	chartData.note.fx[0].emplace(600, kson::Interval{ .length = 0 });

	const auto originalNotes = AllNotes(chartData);

	for (int32 i = 0; i < 100; ++i)
	{
		const Array<std::size_t> noteLanes = MusicGame::MakeSRandomNoteLanes(chartData, 1.0);
		REQUIRE(noteLanes.size() == originalNotes.size());

		kson::ChartData copied = chartData;
		REQUIRE(MusicGame::ApplySRandomNoteLanes(&copied, noteLanes));

		// ノーツの(pulse, length)集合は配置前後で不変
		REQUIRE(AllNotes(copied) == originalNotes);

		// レーン内でノーツが重ならない
		REQUIRE(HasNoOverlap(copied));
	}
}

TEST_CASE("MakeSRandomNoteLanes avoids fast jacks", "[TurnUtil]")
{
	// BPM240の16分間隔(62.5ms)の単ノーツ連打では同一レーンへの連続配置が起こらないことを確認
	kson::ChartData chartData;
	chartData.beat.bpm.emplace(0, 240.0);
	constexpr int32 kNumChips = 32;
	for (int32 i = 0; i < kNumChips; ++i)
	{
		chartData.note.bt[0].emplace(i * (kson::kResolution / 4), kson::Interval{ .length = 0 });
	}

	for (int32 i = 0; i < 100; ++i)
	{
		const Array<std::size_t> noteLanes = MusicGame::MakeSRandomNoteLanes(chartData, 1.0);
		REQUIRE(noteLanes.size() == static_cast<std::size_t>(kNumChips));
		for (std::size_t j = 1U; j < noteLanes.size(); ++j)
		{
			REQUIRE(noteLanes[j] != noteLanes[j - 1U]);
		}
	}
}

TEST_CASE("MakeFXNoteDestinations maps original FX notes to destination lanes", "[TurnUtil]")
{
	kson::ChartData chartData;
	chartData.note.bt[0].emplace(0, kson::Interval{ .length = 0 });
	chartData.note.fx[0].emplace(0, kson::Interval{ .length = 100 });
	chartData.note.fx[1].emplace(200, kson::Interval{ .length = 0 });

	// BT-AのチップをFX-Lへ、FX-LのロングをBT-Cへ、FX-RのチップをBT-Aへ配置
	const Array<std::size_t> noteLanes{ 4, 2, 0 };

	const auto destinations = MusicGame::MakeFXNoteDestinations(chartData, noteLanes);

	// FX-Lのロングの配置先はBT-C
	REQUIRE(destinations[0].size() == 1);
	REQUIRE(destinations[0].at(0) == 2);

	// FX-Rのチップの配置先はBT-A
	REQUIRE(destinations[1].size() == 1);
	REQUIRE(destinations[1].at(200) == 0);
}
