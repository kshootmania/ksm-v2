#include <catch2/catch.hpp>
#include "MusicGame/Graphics/Highway/Note/NoteGraphicsUtils.hpp"

using namespace MusicGame::Graphics::NoteGraphicsUtils;

TEST_CASE("CalcChipNoteColorIndex for default skin", "[NoteGraphicsUtils][CalcChipNoteColorIndex]")
{
	kson::BeatInfo beatInfo;
	const kson::TimingCache timingCache = kson::CreateTimingCache(beatInfo);

	REQUIRE(CalcChipNoteColorIndex(0, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(15, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(30, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(60, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(80, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(120, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(160, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(180, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(240, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(300, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(360, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(480, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(600, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(720, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(840, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(960, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(1920, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
	REQUIRE(CalcChipNoteColorIndex(2880, beatInfo, timingCache, NoteSkinType::kDefault) == 0);
}

TEST_CASE("CalcChipNoteColorIndex for note skin", "[NoteGraphicsUtils][CalcChipNoteColorIndex]")
{
	kson::BeatInfo beatInfo;
	const kson::TimingCache timingCache = kson::CreateTimingCache(beatInfo);

	REQUIRE(CalcChipNoteColorIndex(0, beatInfo, timingCache, NoteSkinType::kNote) == 1);
	REQUIRE(CalcChipNoteColorIndex(15, beatInfo, timingCache, NoteSkinType::kNote) == 8);
	REQUIRE(CalcChipNoteColorIndex(30, beatInfo, timingCache, NoteSkinType::kNote) == 6);
	REQUIRE(CalcChipNoteColorIndex(60, beatInfo, timingCache, NoteSkinType::kNote) == 4);
	REQUIRE(CalcChipNoteColorIndex(80, beatInfo, timingCache, NoteSkinType::kNote) == 3);
	REQUIRE(CalcChipNoteColorIndex(120, beatInfo, timingCache, NoteSkinType::kNote) == 2);
	REQUIRE(CalcChipNoteColorIndex(160, beatInfo, timingCache, NoteSkinType::kNote) == 3);
	REQUIRE(CalcChipNoteColorIndex(180, beatInfo, timingCache, NoteSkinType::kNote) == 4);
	REQUIRE(CalcChipNoteColorIndex(240, beatInfo, timingCache, NoteSkinType::kNote) == 1);
	REQUIRE(CalcChipNoteColorIndex(300, beatInfo, timingCache, NoteSkinType::kNote) == 4);
	REQUIRE(CalcChipNoteColorIndex(360, beatInfo, timingCache, NoteSkinType::kNote) == 2);
	REQUIRE(CalcChipNoteColorIndex(480, beatInfo, timingCache, NoteSkinType::kNote) == 1);
	REQUIRE(CalcChipNoteColorIndex(600, beatInfo, timingCache, NoteSkinType::kNote) == 2);
	REQUIRE(CalcChipNoteColorIndex(720, beatInfo, timingCache, NoteSkinType::kNote) == 1);
	REQUIRE(CalcChipNoteColorIndex(840, beatInfo, timingCache, NoteSkinType::kNote) == 2);
	REQUIRE(CalcChipNoteColorIndex(960, beatInfo, timingCache, NoteSkinType::kNote) == 1);
	REQUIRE(CalcChipNoteColorIndex(1920, beatInfo, timingCache, NoteSkinType::kNote) == 1);
	REQUIRE(CalcChipNoteColorIndex(2880, beatInfo, timingCache, NoteSkinType::kNote) == 1);
}

TEST_CASE("CalcChipNoteColorIndex for note skin with time signature changes", "[NoteGraphicsUtils][CalcChipNoteColorIndex]")
{
	// 0小節目から4/4、2小節目から3/4、4小節目から7/8
	kson::BeatInfo beatInfo;
	beatInfo.bpm.emplace(0, 120.0);
	beatInfo.timeSig.emplace(0, kson::TimeSig{ 4, 4 });
	beatInfo.timeSig.emplace(2, kson::TimeSig{ 3, 4 });
	beatInfo.timeSig.emplace(4, kson::TimeSig{ 7, 8 });
	const kson::TimingCache timingCache = kson::CreateTimingCache(beatInfo);

	// 4/4区間(拍の番号を小節番号として拍子を参照していないこと)
	REQUIRE(CalcChipNoteColorIndex(120, beatInfo, timingCache, NoteSkinType::kNote) == 2);
	REQUIRE(CalcChipNoteColorIndex(1080, beatInfo, timingCache, NoteSkinType::kNote) == 2);

	// 3/4区間(1920〜3360、1小節720)
	// v1と同様に小節の分割数を基準に色分けする
	REQUIRE(CalcChipNoteColorIndex(1920, beatInfo, timingCache, NoteSkinType::kNote) == 1);
	REQUIRE(CalcChipNoteColorIndex(2100, beatInfo, timingCache, NoteSkinType::kNote) == 1);
	REQUIRE(CalcChipNoteColorIndex(2280, beatInfo, timingCache, NoteSkinType::kNote) == 1);
	REQUIRE(CalcChipNoteColorIndex(2010, beatInfo, timingCache, NoteSkinType::kNote) == 2);
	REQUIRE(CalcChipNoteColorIndex(2160, beatInfo, timingCache, NoteSkinType::kNote) == 3);
	REQUIRE(CalcChipNoteColorIndex(2640, beatInfo, timingCache, NoteSkinType::kNote) == 1);

	// 7/8区間(3360〜、1小節840)
	REQUIRE(CalcChipNoteColorIndex(3360, beatInfo, timingCache, NoteSkinType::kNote) == 1);
	REQUIRE(CalcChipNoteColorIndex(3570, beatInfo, timingCache, NoteSkinType::kNote) == 1);
	REQUIRE(CalcChipNoteColorIndex(3465, beatInfo, timingCache, NoteSkinType::kNote) == 2);
	REQUIRE(CalcChipNoteColorIndex(4200, beatInfo, timingCache, NoteSkinType::kNote) == 1);
}
