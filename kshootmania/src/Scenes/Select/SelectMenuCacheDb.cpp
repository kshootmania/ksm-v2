#include "SelectMenuCacheDb.hpp"
#include <SQLiteCpp/SQLiteCpp.h>
#include <filesystem>
#include "Common/Encoding.hpp"
#include "Common/FsUtils.hpp"

namespace
{
	constexpr int32 kSchemaVersion = 1;
	constexpr StringView kDbFileName = U"cache.sqlite3";

	struct SourceFileInfo
	{
		FilePath relativePath;
		int64 mtime = 0;
		uint64 size = 0;
	};

	struct ChartRow
	{
		FilePath songDirectoryRelativePath;
		FilePath sectionDirectoryRelativePath;
		Optional<String> sectionDisplayName;
		bool skipSectionHeading = false;
		bool visibleInDirectoryNameSort = true;
		bool visibleInAllNameSort = true;
		int32 sectionDirectoryIndex = -1;
		int32 songDirectoryIndex = -1;
		int32 chartFileIndex = -1;
		String songSortKey;
		FilePath chartFileRelativePath;
		kson::MetaChartData chartData;
		FolderConfIni folderConfIni;
	};

	std::filesystem::path ToStdPath(FilePathView path)
	{
		return std::filesystem::path{ String{ path }.toUTF32() };
	}

	int64 GetWriteTimeMs(FilePathView path)
	{
		std::error_code ec;
		const auto writeTime = std::filesystem::last_write_time(ToStdPath(path), ec);
		if (ec)
		{
			return 0;
		}
		return std::chrono::duration_cast<std::chrono::milliseconds>(writeTime.time_since_epoch()).count();
	}

	uint64 GetFileSize(FilePathView path)
	{
		std::error_code ec;
		const auto size = std::filesystem::file_size(ToStdPath(path), ec);
		return ec ? 0 : static_cast<uint64>(size);
	}

	uint64 StablePathHash(FilePathView path)
	{
		const std::string utf8Path = String{ path }.toUTF8();
		return static_cast<uint64>(Hash::FNV1a(utf8Path.data(), utf8Path.size()));
	}

	FilePath CacheBasePath()
	{
		const FilePath songsParentPath = FileSystem::ParentPath(FsUtils::SongsDirectoryPath());
		return FileSystem::PathAppend(FileSystem::PathAppend(songsParentPath, U"cache"), U"songs_index");
	}

	FilePath CacheDirectoryPath(FilePathView directoryPath)
	{
		const FilePath fullPath = FileSystem::FullPath(directoryPath);
		FilePath relativePath = FsUtils::RelativePathFromSongsDir(fullPath);
		relativePath.replace(U'\\', U'/');

		if (relativePath.isEmpty() || relativePath == U".")
		{
			relativePath = U"_root";
		}
		else if (relativePath.starts_with(U'/') || relativePath.starts_with(U"../") || relativePath == U".." || relativePath.includes(U':'))
		{
			relativePath = FileSystem::PathAppend(U"_external", Format(StablePathHash(fullPath)));
		}

		return FileSystem::PathAppend(CacheBasePath(), relativePath);
	}

	FilePath DbPath(FilePathView directoryPath)
	{
		return FileSystem::PathAppend(CacheDirectoryPath(directoryPath), kDbFileName);
	}

	Array<FilePath> GetSubDirectories(FilePathView path)
	{
		Array<FilePath> directories =
			FileSystem::DirectoryContents(path, Recursive::No)
				.filter([](FilePathView p) { return FileSystem::IsDirectory(p); });

		directories.sort_by([](const FilePath& a, const FilePath& b)
		{
			return FsUtils::DirectoryNameByDirectoryPath(a).lowercased() < FsUtils::DirectoryNameByDirectoryPath(b).lowercased();
		});

		return directories;
	}

	bool IsSourceFile(FilePathView path)
	{
		if (!FileSystem::IsFile(path))
		{
			return false;
		}

		const String filename = FileSystem::FileName(path);
		if (filename == kDbFileName)
		{
			return false;
		}

		const String extension = FileSystem::Extension(path).lowercased();
		return extension == U"ksh"
			|| extension == U"kson"
			|| extension == U"kco"
			|| filename.lowercased() == U"foldername.csv"
			|| filename.lowercased() == U"conf.ini";
	}

	Array<SourceFileInfo> CollectSourceFiles(FilePathView baseDirectoryPath)
	{
		Array<SourceFileInfo> files;
		for (const auto& path : FileSystem::DirectoryContents(baseDirectoryPath, Recursive::Yes))
		{
			if (!IsSourceFile(path))
			{
				continue;
			}

			files.push_back(SourceFileInfo{
				.relativePath = FileSystem::RelativePath(path, baseDirectoryPath),
				.mtime = GetWriteTimeMs(path),
				.size = GetFileSize(path),
			});
		}

		files.sort_by([](const SourceFileInfo& a, const SourceFileInfo& b)
		{
			return a.relativePath < b.relativePath;
		});
		return files;
	}

	int64 LatestMtime(const Array<SourceFileInfo>& sourceFiles)
	{
		int64 latest = 0;
		for (const auto& sourceFile : sourceFiles)
		{
			latest = Max(latest, sourceFile.mtime);
		}
		return latest;
	}

	// foldername.csvを読み込んでフォルダ名から表示名への対応関係を取得
	// (キー"*"は全フォルダに対して適用される表示名を示す)
	HashTable<String, String> LoadFolderNameTable(FilePathView directoryPath)
	{
		HashTable<String, String> folderNameTable;
		const FilePath csvPath = FileSystem::PathAppend(directoryPath, U"foldername.csv");

		if (!FileSystem::IsFile(csvPath))
		{
			return folderNameTable;
		}

		const Array<String> lines = Encoding::ReadTextFileLinesShiftJISOrUTF8BasedOnBOM(csvPath);
		for (const String& line : lines)
		{
			const Array<String> parts = line.split(U',');
			if (parts.size() < 2)
			{
				continue;
			}

			auto removeQuotes = [](String str) -> String
			{
				str = str.trimmed();
				if (str.length() >= 2 && str.starts_with(U'"') && str.ends_with(U'"'))
				{
					return str.substr(1, str.length() - 2);
				}
				return str;
			};

			const String folderName = removeQuotes(parts[0]);
			const String displayName = removeQuotes(parts[1]);
			if (!folderName.isEmpty())
			{
				// displayNameが空文字列の場合はサブフォルダ見出しを非表示化
				folderNameTable[folderName] = displayName;
			}
		}

		return folderNameTable;
	}

	// foldername.csvによる置換後の表示フォルダ名を取得(対応関係がない場合はnone)
	Optional<String> GetDisplayNameFromFolderNameTable(const HashTable<String, String>& folderNameTable, const String& folderName)
	{
		if (const auto it = folderNameTable.find(folderName); it != folderNameTable.end())
		{
			return it->second;
		}

		// "*"による全フォルダ置換があれば使用
		if (const auto it = folderNameTable.find(U"*"); it != folderNameTable.end())
		{
			return it->second;
		}

		return none;
	}

	bool AppendChartRows(
		Array<ChartRow>* pRows,
		FilePathView baseDirectoryPath,
		FilePathView songDirPath,
		FilePathView sectionDirPath,
		const Optional<String>& sectionDisplayName,
		bool skipSectionHeading,
		bool visibleInDirectoryNameSort,
		bool visibleInAllNameSort,
		int32 sectionDirectoryIndex,
		int32 songDirectoryIndex)
	{
		bool chartExists = false;
		const Array<FilePath> chartFiles = FsUtils::GetChartFilePathsPreferringKson(songDirPath);
		for (size_t chartFileIndex = 0; chartFileIndex < chartFiles.size(); ++chartFileIndex)
		{
			const auto& chartFilePath = chartFiles[chartFileIndex];
			if (!FsUtils::HasChartExtension(chartFilePath))
			{
				continue;
			}

			kson::MetaChartData chartData = FsUtils::HasKsonExtension(chartFilePath)
				? kson::LoadKsonMetaChartData(chartFilePath.toUTF8())
				: kson::LoadKshMetaChartData(chartFilePath.toUTF8());

			if (chartData.error != kson::ErrorType::None)
			{
				Logger << U"[ksm warning] SelectMenuCacheDb: Chart Loading Error (error:'{}', chartFilePath:'{}')"_fmt(Unicode::FromUTF8(kson::GetErrorString(chartData.error)), chartFilePath);
				continue;
			}

			int32 difficultyIdx = chartData.meta.difficulty.idx;
			if (difficultyIdx < 0 || kNumDifficulties <= difficultyIdx)
			{
				Logger << U"[ksm warning] SelectMenuCacheDb: Difficulty index out of range (difficultyIdx:{}, chartFilePath:'{}')"_fmt(difficultyIdx, chartFilePath);
				difficultyIdx = kNumDifficulties - 1;
				chartData.meta.difficulty.idx = difficultyIdx;
			}

			pRows->push_back(ChartRow{
				.songDirectoryRelativePath = FileSystem::RelativePath(songDirPath, baseDirectoryPath),
				.sectionDirectoryRelativePath = sectionDirPath.isEmpty() ? FilePath{} : FileSystem::RelativePath(sectionDirPath, baseDirectoryPath),
				.sectionDisplayName = sectionDisplayName,
				.skipSectionHeading = skipSectionHeading,
				.visibleInDirectoryNameSort = visibleInDirectoryNameSort,
				.visibleInAllNameSort = visibleInAllNameSort,
				.sectionDirectoryIndex = sectionDirectoryIndex,
				.songDirectoryIndex = songDirectoryIndex,
				.chartFileIndex = static_cast<int32>(chartFileIndex),
				.songSortKey = FsUtils::DirectoryNameByDirectoryPath(songDirPath).lowercased(),
				.chartFileRelativePath = FileSystem::RelativePath(chartFilePath, baseDirectoryPath),
				.chartData = chartData,
				.folderConfIni = FolderConfIni::Load(chartFilePath),
			});
			chartExists = true;
		}
		return chartExists;
	}

	Array<ChartRow> CollectChartRows(FilePathView baseDirectoryPath)
	{
		Array<ChartRow> rows;
		const HashTable<String, String> folderNameTable = LoadFolderNameTable(baseDirectoryPath);

		const Array<FilePath> songDirectories = GetSubDirectories(baseDirectoryPath);
		for (size_t songDirectoryIndex = 0; songDirectoryIndex < songDirectories.size(); ++songDirectoryIndex)
		{
			const auto& songDirectory = songDirectories[songDirectoryIndex];
			const Array<FilePath> subDirs = GetSubDirectories(songDirectory);
			const bool directChartExists = AppendChartRows(
				&rows,
				baseDirectoryPath,
				songDirectory,
				U"",
				none,
				false,
				true,
				subDirs.isEmpty(),
				-1,
				static_cast<int32>(songDirectoryIndex));

			for (size_t subDirIndex = 0; subDirIndex < subDirs.size(); ++subDirIndex)
			{
				const auto& subDir = subDirs[subDirIndex];
				const String folderName = FsUtils::DirectoryNameByDirectoryPath(songDirectory);
				const Optional<String> displayName = GetDisplayNameFromFolderNameTable(folderNameTable, folderName);
				const bool skipHeading = displayName.has_value() && displayName->isEmpty();

				AppendChartRows(
					&rows,
					baseDirectoryPath,
					subDir,
					songDirectory,
					displayName,
					directChartExists ? true : skipHeading,
					!directChartExists,
					true,
					static_cast<int32>(songDirectoryIndex),
					static_cast<int32>(subDirIndex));
			}
		}

		return rows;
	}

	void CreateSchema(SQLite::Database& db)
	{
		db.exec("CREATE TABLE IF NOT EXISTS cache_meta (key TEXT PRIMARY KEY, value TEXT NOT NULL)");
		db.exec("CREATE TABLE IF NOT EXISTS source_files (relative_path TEXT PRIMARY KEY, mtime_ms INTEGER NOT NULL, size INTEGER NOT NULL)");
		db.exec(
			"CREATE TABLE IF NOT EXISTS charts ("
			"chart_path TEXT PRIMARY KEY,"
			"song_dir TEXT NOT NULL,"
			"section_dir TEXT NOT NULL,"
			"section_display_name TEXT,"
			"skip_section_heading BOOLEAN NOT NULL,"
			"visible_in_directory_name_sort BOOLEAN NOT NULL,"
			"visible_in_all_name_sort BOOLEAN NOT NULL,"
			"section_directory_index INTEGER NOT NULL,"
			"song_directory_index INTEGER NOT NULL,"
			"chart_file_index INTEGER NOT NULL,"
			"song_sort_key TEXT NOT NULL,"
			"difficulty_idx INTEGER NOT NULL,"
			"level INTEGER NOT NULL,"
			"title TEXT NOT NULL,"
			"title_translit TEXT NOT NULL,"
			"title_img_filename TEXT NOT NULL,"
			"artist TEXT NOT NULL,"
			"artist_translit TEXT NOT NULL,"
			"artist_img_filename TEXT NOT NULL,"
			"chart_author TEXT NOT NULL,"
			"disp_bpm TEXT NOT NULL,"
			"std_bpm REAL NOT NULL,"
			"jacket_filename TEXT NOT NULL,"
			"jacket_author TEXT NOT NULL,"
			"icon_filename TEXT NOT NULL,"
			"information TEXT NOT NULL,"
			"bgm_filename TEXT NOT NULL,"
			"bgm_vol REAL NOT NULL,"
			"preview_offset INTEGER NOT NULL,"
			"preview_duration INTEGER NOT NULL,"
			"folder_volume_scale REAL NOT NULL,"
			"folder_offset_ms INTEGER NOT NULL,"
			"folder_tweet_option TEXT NOT NULL"
			")");
	}

	void SetMeta(SQLite::Database& db, StringView key, StringView value)
	{
		SQLite::Statement query(db, "INSERT OR REPLACE INTO cache_meta(key, value) VALUES(?, ?)");
		int32 columnIndex = 1;
		query.bind(columnIndex++, key.toUTF8());
		query.bind(columnIndex++, value.toUTF8());
		query.exec();
	}

	Optional<String> GetMeta(SQLite::Database& db, StringView key)
	{
		SQLite::Statement query(db, "SELECT value FROM cache_meta WHERE key = ?");
		int32 columnIndex = 1;
		query.bind(columnIndex++, key.toUTF8());
		if (!query.executeStep())
		{
			return none;
		}
		return Unicode::FromUTF8(query.getColumn(0).getString());
	}

	bool IsCacheValid(SQLite::Database& db, FilePathView baseDirectoryPath, const Array<SourceFileInfo>& sourceFiles)
	{
		const Optional<String> schemaVersion = GetMeta(db, U"schema_version");
		const Optional<String> sourceFileCount = GetMeta(db, U"source_file_count");
		const Optional<String> latestSourceMtime = GetMeta(db, U"latest_source_mtime_ms");
		if (!schemaVersion || !sourceFileCount || !latestSourceMtime)
		{
			return false;
		}
		if (ParseOr<int32>(*schemaVersion, 0) != kSchemaVersion)
		{
			return false;
		}
		if (ParseOr<int32>(*sourceFileCount, -1) != static_cast<int32>(sourceFiles.size()))
		{
			return false;
		}
		if (ParseOr<int64>(*latestSourceMtime, -1) != LatestMtime(sourceFiles))
		{
			return false;
		}

		SQLite::Statement query(db, "SELECT relative_path, mtime_ms, size FROM source_files ORDER BY relative_path");
		std::size_t index = 0;
		while (query.executeStep())
		{
			if (index >= sourceFiles.size())
			{
				return false;
			}
			if (Unicode::FromUTF8(query.getColumn(0).getString()) != sourceFiles[index].relativePath
				|| query.getColumn(1).getInt64() != sourceFiles[index].mtime
				|| static_cast<uint64>(query.getColumn(2).getInt64()) != sourceFiles[index].size)
			{
				return false;
			}
			++index;
		}

		return index == sourceFiles.size() && FileSystem::FullPath(baseDirectoryPath) == GetMeta(db, U"directory_path").value_or(U"");
	}

	void Rebuild(SQLite::Database& db, FilePathView baseDirectoryPath, const Array<SourceFileInfo>& sourceFiles)
	{
		FolderConfIni::ClearCache();
		const Array<ChartRow> chartRows = CollectChartRows(baseDirectoryPath);

		SQLite::Transaction transaction(db);
		db.exec("DROP TABLE IF EXISTS cache_meta");
		db.exec("DROP TABLE IF EXISTS source_files");
		db.exec("DROP TABLE IF EXISTS charts");
		CreateSchema(db);

		SetMeta(db, U"schema_version", Format(kSchemaVersion));
		SetMeta(db, U"directory_path", FileSystem::FullPath(baseDirectoryPath));
		SetMeta(db, U"source_file_count", Format(sourceFiles.size()));
		SetMeta(db, U"latest_source_mtime_ms", Format(LatestMtime(sourceFiles)));

		{
			SQLite::Statement query(db, "INSERT INTO source_files(relative_path, mtime_ms, size) VALUES(?, ?, ?)");
			for (const auto& sourceFile : sourceFiles)
			{
				int32 columnIndex = 1;
				query.bind(columnIndex++, sourceFile.relativePath.toUTF8());
				query.bind(columnIndex++, sourceFile.mtime);
				query.bind(columnIndex++, static_cast<int64>(sourceFile.size));
				query.exec();
				query.reset();
			}
		}

		{
			SQLite::Statement query(db,
				"INSERT INTO charts("
				"chart_path, song_dir, section_dir, section_display_name, skip_section_heading,"
				"visible_in_directory_name_sort, visible_in_all_name_sort, section_directory_index, song_directory_index, chart_file_index, song_sort_key,"
				"difficulty_idx, level,"
				"title, title_translit, title_img_filename, artist, artist_translit, artist_img_filename, chart_author,"
				"disp_bpm, std_bpm, jacket_filename, jacket_author, icon_filename, information,"
				"bgm_filename, bgm_vol, preview_offset, preview_duration, folder_volume_scale, folder_offset_ms, folder_tweet_option"
				") VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

			for (const auto& row : chartRows)
			{
				const auto& meta = row.chartData.meta;
				const auto& bgm = row.chartData.audio.bgm;
				int32 columnIndex = 1;
				query.bind(columnIndex++, row.chartFileRelativePath.toUTF8());
				query.bind(columnIndex++, row.songDirectoryRelativePath.toUTF8());
				query.bind(columnIndex++, row.sectionDirectoryRelativePath.toUTF8());
				if (row.sectionDisplayName)
				{
					query.bind(columnIndex++, row.sectionDisplayName->toUTF8());
				}
				else
				{
					query.bind(columnIndex++);
				}
				query.bind(columnIndex++, row.skipSectionHeading ? 1 : 0);
				query.bind(columnIndex++, row.visibleInDirectoryNameSort ? 1 : 0);
				query.bind(columnIndex++, row.visibleInAllNameSort ? 1 : 0);
				query.bind(columnIndex++, row.sectionDirectoryIndex);
				query.bind(columnIndex++, row.songDirectoryIndex);
				query.bind(columnIndex++, row.chartFileIndex);
				query.bind(columnIndex++, row.songSortKey.toUTF8());
				query.bind(columnIndex++, meta.difficulty.idx);
				query.bind(columnIndex++, meta.level);
				query.bind(columnIndex++, meta.title);
				query.bind(columnIndex++, meta.titleTranslit);
				query.bind(columnIndex++, meta.titleImgFilename);
				query.bind(columnIndex++, meta.artist);
				query.bind(columnIndex++, meta.artistTranslit);
				query.bind(columnIndex++, meta.artistImgFilename);
				query.bind(columnIndex++, meta.chartAuthor);
				query.bind(columnIndex++, meta.dispBPM);
				query.bind(columnIndex++, meta.stdBPM);
				query.bind(columnIndex++, meta.jacketFilename);
				query.bind(columnIndex++, meta.jacketAuthor);
				query.bind(columnIndex++, meta.iconFilename);
				query.bind(columnIndex++, meta.information);
				query.bind(columnIndex++, bgm.filename);
				query.bind(columnIndex++, bgm.vol);
				query.bind(columnIndex++, bgm.preview.offset);
				query.bind(columnIndex++, bgm.preview.duration);
				query.bind(columnIndex++, row.folderConfIni.volumeScale);
				query.bind(columnIndex++, row.folderConfIni.offsetMs);
				query.bind(columnIndex++, row.folderConfIni.tweetOption.toUTF8());
				query.exec();
				query.reset();
			}
		}

		transaction.commit();
	}

	struct ChartRowResult
	{
		std::shared_ptr<const SelectChartInfo> chartInfo;
		int32 difficultyIdx = 0;
		String songSortKey;
		FilePath songDirectoryPath;
		FilePath sectionDirectoryPath;
		Optional<String> sectionDisplayName;
		bool skipSectionHeading = false;
		bool visibleInDirectoryNameSort = true;
		bool visibleInAllNameSort = true;
		int32 sectionDirectoryIndex = -1;
		int32 songDirectoryIndex = -1;
	};

	Array<ChartRowResult> ReadChartRows(SQLite::Database& db, FilePathView baseDirectoryPath, StringView whereClause, StringView orderByClause)
	{
		Array<ChartRowResult> rows;

		const std::string sql = "SELECT * FROM charts "
			+ whereClause.toUTF8()
			+ " ORDER BY "
			+ orderByClause.toUTF8();
		SQLite::Statement query(db, sql);
		while (query.executeStep())
		{
			kson::MetaChartData chartData;
			auto& meta = chartData.meta;
			meta.difficulty.idx = query.getColumn("difficulty_idx").getInt();
			meta.level = query.getColumn("level").getInt();
			meta.title = query.getColumn("title").getString();
			meta.titleTranslit = query.getColumn("title_translit").getString();
			meta.titleImgFilename = query.getColumn("title_img_filename").getString();
			meta.artist = query.getColumn("artist").getString();
			meta.artistTranslit = query.getColumn("artist_translit").getString();
			meta.artistImgFilename = query.getColumn("artist_img_filename").getString();
			meta.chartAuthor = query.getColumn("chart_author").getString();
			meta.dispBPM = query.getColumn("disp_bpm").getString();
			meta.stdBPM = query.getColumn("std_bpm").getDouble();
			meta.jacketFilename = query.getColumn("jacket_filename").getString();
			meta.jacketAuthor = query.getColumn("jacket_author").getString();
			meta.iconFilename = query.getColumn("icon_filename").getString();
			meta.information = query.getColumn("information").getString();
			chartData.audio.bgm.filename = query.getColumn("bgm_filename").getString();
			chartData.audio.bgm.vol = query.getColumn("bgm_vol").getDouble();
			chartData.audio.bgm.preview.offset = query.getColumn("preview_offset").getInt();
			chartData.audio.bgm.preview.duration = query.getColumn("preview_duration").getInt();

			FolderConfIni folderConfIni;
			folderConfIni.volumeScale = query.getColumn("folder_volume_scale").getDouble();
			folderConfIni.offsetMs = query.getColumn("folder_offset_ms").getInt();
			folderConfIni.tweetOption = Unicode::FromUTF8(query.getColumn("folder_tweet_option").getString());

			const FilePath chartPath = FileSystem::PathAppend(baseDirectoryPath, Unicode::FromUTF8(query.getColumn("chart_path").getString()));
			const FilePath songDirRel = Unicode::FromUTF8(query.getColumn("song_dir").getString());
			const FilePath sectionDirRel = Unicode::FromUTF8(query.getColumn("section_dir").getString());

			rows.push_back(ChartRowResult{
				.chartInfo = std::make_shared<SelectChartInfo>(chartPath, chartData, folderConfIni),
				.difficultyIdx = meta.difficulty.idx,
				.songSortKey = Unicode::FromUTF8(query.getColumn("song_sort_key").getString()),
				.songDirectoryPath = FileSystem::PathAppend(baseDirectoryPath, songDirRel),
				.sectionDirectoryPath = sectionDirRel.isEmpty() ? FilePath{} : FileSystem::PathAppend(baseDirectoryPath, sectionDirRel),
				.sectionDisplayName = query.getColumn("section_display_name").isNull()
					? Optional<String>{}
					: Optional<String>{ Unicode::FromUTF8(query.getColumn("section_display_name").getString()) },
				.skipSectionHeading = query.getColumn("skip_section_heading").getInt() != 0,
				.visibleInDirectoryNameSort = query.getColumn("visible_in_directory_name_sort").getInt() != 0,
				.visibleInAllNameSort = query.getColumn("visible_in_all_name_sort").getInt() != 0,
				.sectionDirectoryIndex = query.getColumn("section_directory_index").getInt(),
				.songDirectoryIndex = query.getColumn("song_directory_index").getInt(),
			});
		}

		return rows;
	}

	Array<ChartRowResult> LoadDirectoryRows(FilePathView directoryPath, bool forceRebuild, StringView whereClause, StringView orderByClause, StringView fnName)
	{
		const FilePath baseDirectoryPath = FileSystem::FullPath(directoryPath);
		const FilePath dbPath = DbPath(baseDirectoryPath);

		const auto loadOnce = [&](bool rebuild) -> Array<ChartRowResult>
		{
			const Array<SourceFileInfo> sourceFiles = CollectSourceFiles(baseDirectoryPath);
			FileSystem::CreateDirectories(FileSystem::ParentPath(dbPath));
			SQLite::Database db(dbPath.toUTF8(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
			CreateSchema(db);
			if (rebuild || !IsCacheValid(db, baseDirectoryPath, sourceFiles))
			{
				Rebuild(db, baseDirectoryPath, sourceFiles);
			}
			return ReadChartRows(db, baseDirectoryPath, whereClause, orderByClause);
		};

		try
		{
			return loadOnce(forceRebuild);
		}
		catch (const std::exception& e)
		{
			Logger << U"[ksm warning] SelectMenuCacheDb::{} failed. Fallback to rebuild. (path:'{}', error:'{}')"_fmt(fnName, directoryPath, Unicode::FromUTF8(e.what()));
			FileSystem::Remove(dbPath);
			return loadOnce(true);
		}
	}

	Array<SelectCachedSongEntry> GroupRowsAsSongEntries(const Array<ChartRowResult>& rows)
	{
		Array<SelectCachedSongEntry> entries;
		HashTable<FilePath, size_t> songDirToEntryIndex;

		for (const auto& row : rows)
		{
			auto [it, inserted] = songDirToEntryIndex.emplace(row.songDirectoryPath, entries.size());
			if (inserted)
			{
				entries.push_back(SelectCachedSongEntry{
					.song = SelectCachedSong{
						.songDirectoryPath = row.songDirectoryPath,
						.sectionDirectoryPath = row.sectionDirectoryPath,
						.sectionDisplayName = row.sectionDisplayName,
						.skipSectionHeading = row.skipSectionHeading,
						.visibleInDirectoryNameSort = row.visibleInDirectoryNameSort,
						.visibleInAllNameSort = row.visibleInAllNameSort,
						.sectionDirectoryIndex = row.sectionDirectoryIndex,
						.songDirectoryIndex = row.songDirectoryIndex,
						.songSortKey = row.songSortKey,
					},
				});
			}

			auto& song = entries[it->second].song;
			const int32 d = row.difficultyIdx;
			if (0 <= d && d < kNumDifficulties && song.chartInfos[d] == nullptr)
			{
				song.chartInfos[d] = row.chartInfo;
			}
			else if (0 <= d && d < kNumDifficulties)
			{
				SelectCachedSong duplicateSong{
					.songDirectoryPath = row.songDirectoryPath,
					.sectionDirectoryPath = row.sectionDirectoryPath,
					.sectionDisplayName = row.sectionDisplayName,
					.skipSectionHeading = row.skipSectionHeading,
					.visibleInDirectoryNameSort = row.visibleInDirectoryNameSort,
					.visibleInAllNameSort = row.visibleInAllNameSort,
					.sectionDirectoryIndex = row.sectionDirectoryIndex,
					.songDirectoryIndex = row.songDirectoryIndex,
					.songSortKey = row.songSortKey,
				};
				duplicateSong.chartInfos[d] = row.chartInfo;
				entries.push_back(SelectCachedSongEntry{
					.song = std::move(duplicateSong),
					.isSingleChartItem = true,
				});
			}
		}

		return entries;
	}

	Array<SelectCachedChartEntry> RowsToChartEntries(const Array<ChartRowResult>& rows)
	{
		Array<SelectCachedChartEntry> entries;
		entries.reserve(rows.size());
		for (const auto& row : rows)
		{
			entries.push_back(SelectCachedChartEntry{
				.chartInfo = row.chartInfo,
				.difficultyIdx = row.difficultyIdx,
				.songSortKey = row.songSortKey,
			});
		}
		return entries;
	}
}

Array<SelectCachedSongEntry> SelectMenuCacheDb::LoadDirectoryForNameSort(FilePathView directoryPath, bool forceRebuild)
{
	return GroupRowsAsSongEntries(LoadDirectoryRows(
		directoryPath,
		forceRebuild,
		U"WHERE visible_in_directory_name_sort != 0",
		U"CASE WHEN section_directory_index < 0 THEN 0 ELSE 1 END, section_directory_index, song_directory_index, chart_file_index",
		U"LoadDirectoryForNameSort"));
}

Array<SelectCachedSongEntry> SelectMenuCacheDb::LoadDirectoryForAllNameSort(FilePathView directoryPath, bool forceRebuild)
{
	return GroupRowsAsSongEntries(LoadDirectoryRows(
		directoryPath,
		forceRebuild,
		U"WHERE visible_in_all_name_sort != 0",
		U"song_sort_key, section_directory_index, song_directory_index, chart_file_index",
		U"LoadDirectoryForAllNameSort"));
}

Array<SelectCachedChartEntry> SelectMenuCacheDb::LoadDirectoryForLevelSort(FilePathView directoryPath, bool forceRebuild)
{
	return RowsToChartEntries(LoadDirectoryRows(
		directoryPath,
		forceRebuild,
		U"",
		U"section_directory_index, song_directory_index, chart_file_index",
		U"LoadDirectoryForLevelSort"));
}

void SelectMenuCacheDb::Invalidate(FilePathView directoryPath)
{
	if (!directoryPath.empty() && FileSystem::IsDirectory(directoryPath))
	{
		FileSystem::Remove(DbPath(FileSystem::FullPath(directoryPath)));
	}
}

void SelectMenuCacheDb::InvalidateAll()
{
	for (const auto& directory : GetSubDirectories(FsUtils::SongsDirectoryPath()))
	{
		Invalidate(directory);
	}
}
