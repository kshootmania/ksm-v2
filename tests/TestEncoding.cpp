#include <catch2/catch.hpp>
#include <filesystem>
#include <fstream>
#include "Common/Encoding.hpp"

namespace
{
	void WriteBytes(const std::filesystem::path& path, std::string_view bytes)
	{
		std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
		ofs.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
	}

	FilePath ToFilePath(const std::filesystem::path& path)
	{
		const std::u8string u8str = path.u8string();
		return Unicode::FromUTF8(std::string_view(reinterpret_cast<const char*>(u8str.data()), u8str.size()));
	}

	std::filesystem::path MakeTempFilePath()
	{
		static int32 counter = 0;
		return std::filesystem::temp_directory_path() /
			("ksm_encoding_test_" + std::to_string(++counter) + ".txt");
	}
}

TEST_CASE("ReadTextFileLinesShiftJISOrUTF8BasedOnBOM reads UTF-8 with BOM", "[Encoding]")
{
	const auto path = MakeTempFilePath();
	const std::string text = "\xEF\xBB\xBF\xE6\xA5\xBD\xE6\x9B\xB2/\xE3\x83\x86\xE3\x82\xB9\xE3\x83\x88.ksh\r\n";
	WriteBytes(path, text);

	const Array<String> lines = Encoding::ReadTextFileLinesShiftJISOrUTF8BasedOnBOM(ToFilePath(path));
	std::filesystem::remove(path);

	REQUIRE(lines.size() == 1);
	REQUIRE(lines[0] == U"楽曲/テスト.ksh");
}

TEST_CASE("ReadTextFileLinesShiftJISOrUTF8BasedOnBOM reads Shift-JIS without BOM", "[Encoding]")
{
	const auto path = MakeTempFilePath();
	// Shift-JISの「テスト.ksh」
	const std::string text = "\x83\x65\x83\x58\x83\x67.ksh\n";
	WriteBytes(path, text);

	const Array<String> lines = Encoding::ReadTextFileLinesShiftJISOrUTF8BasedOnBOM(ToFilePath(path));
	std::filesystem::remove(path);

	REQUIRE(lines.size() == 1);
	REQUIRE(lines[0] == U"テスト.ksh");
}
