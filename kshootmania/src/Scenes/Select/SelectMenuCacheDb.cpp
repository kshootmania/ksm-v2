#include "SelectMenuCacheDb.hpp"
#include <SQLiteCpp/SQLiteCpp.h>
#include <filesystem>
#include "Common/Encoding.hpp"
#include "Common/FsUtils.hpp"

namespace
{
	constexpr int32 kSchemaVersion = 2;
	constexpr StringView kDbFileName = U"cache.sqlite3";

	struct SourceFileInfo
	{
		FilePath relativePath;
		int64 mtime = 0;
		uint64 size = 0;

		bool operator==(const SourceFileInfo&) const = default;
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

	// パスのnative()の文字列をStringに変換
	// (不正なUTF-8を含むファイル名でもpath::u32string()のように例外にならないよう、FileSystem::DirectoryContentsと同じ変換を使う)
	String FromNativePath(const std::filesystem::path::string_type& nativePath)
	{
#ifdef _WIN32
		return Unicode::FromWstring(nativePath);
#else
		return Unicode::FromUTF8(nativePath);
#endif
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
		else
		{
			bool isOutsideSongsDir = relativePath.starts_with(U'/') || relativePath.starts_with(U"../") || relativePath == U"..";
#ifdef _WIN32
			isOutsideSongsDir = isOutsideSongsDir || relativePath.includes(U':');
#endif
			if (isOutsideSongsDir)
			{
				relativePath = FileSystem::PathAppend(U"_external", Format(StablePathHash(fullPath)));
			}
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

	bool IsSourceFile(const String& filename)
	{
		if (filename == kDbFileName)
		{
			return false;
		}

		const String lowercased = filename.lowercased();
		return lowercased.ends_with(U".ksh")
			|| lowercased.ends_with(U".kson")
			|| lowercased.ends_with(U".kco")
			|| lowercased == U"foldername.csv"
			|| lowercased == U"conf.ini";
	}

	Array<SourceFileInfo> CollectSourceFiles(FilePathView baseDirectoryPath)
	{
		Array<SourceFileInfo> files;

		// ディレクトリ列挙で得た情報をそのまま使い、ファイル毎の情報取得の回数を減らす
		const std::filesystem::path basePath = ToStdPath(FileSystem::FullPath(baseDirectoryPath));
		const std::size_t basePathLength = basePath.native().size();
#ifdef _WIN32
		// Windows版のFileSystem::DirectoryContentsと同様に、ジャンクション・シンボリックリンクのディレクトリの中のファイルも取得する
		constexpr auto kDirectoryOptions = std::filesystem::directory_options::skip_permission_denied | std::filesystem::directory_options::follow_directory_symlink;
#else
		constexpr auto kDirectoryOptions = std::filesystem::directory_options::skip_permission_denied;
#endif
		std::error_code ec;
		for (std::filesystem::recursive_directory_iterator it(basePath, kDirectoryOptions, ec), end; it != end; it.increment(ec))
		{
			if (ec)
			{
				break;
			}

			const std::filesystem::directory_entry& entry = *it;
			if (!entry.is_regular_file(ec))
			{
				continue;
			}

			const std::filesystem::path& path = entry.path();
			const String filename = FromNativePath(path.filename().native());
			if (!IsSourceFile(filename))
			{
				continue;
			}

			String relativePath = FromNativePath(path.native().substr(basePathLength));
			relativePath.replace(U'\\', U'/');
			if (relativePath.starts_with(U'/'))
			{
				relativePath.erase(0, 1);
			}

			const auto writeTime = entry.last_write_time(ec);
			const int64 mtime = ec ? 0 : std::chrono::duration_cast<std::chrono::milliseconds>(writeTime.time_since_epoch()).count();
			ec.clear();
			const auto size = entry.file_size(ec);
			ec.clear();

			files.push_back(SourceFileInfo{
				.relativePath = std::move(relativePath),
				.mtime = mtime,
				.size = static_cast<uint64>(size),
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

	// 譜面ファイルの相対パスを取得(FileSystem::RelativePath(chartFilePath, baseDirectoryPath)と同じ結果を返す)
	FilePath ChartFileRelativePath(FilePathView chartFilePath, FilePathView songDirPath, FilePathView songDirectoryRelativePath, FilePathView baseDirectoryPath)
	{
		// 譜面ファイルのパスはリンク先のパスになっている場合があるため、楽曲フォルダ直下にあるかを確認
		const bool endsWithSlash = songDirPath.ends_with(U'/');
		const size_t filenamePos = songDirPath.size() + (endsWithSlash ? 0 : 1);
		const bool isInSongDir = chartFilePath.size() > filenamePos && chartFilePath.starts_with(songDirPath) && (endsWithSlash || chartFilePath[songDirPath.size()] == U'/');
		if (isInSongDir && !songDirectoryRelativePath.isEmpty() && songDirectoryRelativePath != U"./")
		{
			const StringView filename = chartFilePath.substr(filenamePos);
			std::error_code ec;
			if (!filename.includes(U'/') && !std::filesystem::is_symlink(ToStdPath(chartFilePath), ec) && !ec)
			{
				// シンボリックリンクでないファイルは、楽曲フォルダの相対パスにファイル名を追加したものを相対パスとして使用
				FilePath relativePath{ songDirectoryRelativePath };
				if (!relativePath.ends_with(U'/'))
				{
					relativePath.push_back(U'/');
				}
				relativePath.append(filename);
				return relativePath;
			}
		}

		return FileSystem::RelativePath(chartFilePath, baseDirectoryPath);
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
		if (chartFiles.isEmpty())
		{
			return false;
		}

		// 楽曲フォルダ内の譜面で共通の値(conf.iniはパッケージフォルダ単位のため、最初の譜面から取得)
		const FilePath songDirectoryRelativePath = FileSystem::RelativePath(songDirPath, baseDirectoryPath);
		const FilePath sectionDirectoryRelativePath = sectionDirPath.isEmpty() ? FilePath{} : FileSystem::RelativePath(sectionDirPath, baseDirectoryPath);
		const FolderConfIni folderConfIni = FolderConfIni::Load(chartFiles.front());

		for (size_t chartFileIndex = 0; chartFileIndex < chartFiles.size(); ++chartFileIndex)
		{
			const auto& chartFilePath = chartFiles[chartFileIndex];
			if (!FsUtils::HasChartExtension(chartFilePath))
			{
				continue;
			}

			kson::MetaChartData chartData = SelectChartInfo::LoadMetaChartData(chartFilePath);

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
				.songDirectoryRelativePath = songDirectoryRelativePath,
				.sectionDirectoryRelativePath = sectionDirectoryRelativePath,
				.sectionDisplayName = sectionDisplayName,
				.skipSectionHeading = skipSectionHeading,
				.visibleInDirectoryNameSort = visibleInDirectoryNameSort,
				.visibleInAllNameSort = visibleInAllNameSort,
				.sectionDirectoryIndex = sectionDirectoryIndex,
				.songDirectoryIndex = songDirectoryIndex,
				.chartFileIndex = static_cast<int32>(chartFileIndex),
				.songSortKey = FsUtils::DirectoryNameByDirectoryPath(songDirPath).lowercased(),
				.chartFileRelativePath = ChartFileRelativePath(chartFilePath, songDirPath, songDirectoryRelativePath, baseDirectoryPath),
				.chartData = chartData,
				.folderConfIni = folderConfIni,
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
		// DBファイルを新規作成した直後はテーブルが存在しない
		if (!db.tableExists("cache_meta"))
		{
			return false;
		}

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
		int32 chartFileIndex = -1;
	};

	// 読み込み済みのディレクトリの内容
	// (選曲画面に戻るたびにDBを読み直さないよう、キャッシュの元になるファイルの一覧が変わっていなければ再利用する)
	struct MemoryCacheEntry
	{
		Array<SourceFileInfo> sourceFiles;
		Array<ChartRowResult> rows;
	};

	HashTable<FilePath, MemoryCacheEntry> s_memoryCache;

	// ディレクトリ内の全譜面の行を、セクション・楽曲フォルダ・譜面ファイルの順で取得
	Array<ChartRowResult> ReadChartRows(SQLite::Database& db, FilePathView baseDirectoryPath)
	{
		Array<ChartRowResult> rows;

		// 列名での検索を避けるため、列の順番を固定して取得する
		const std::string sql = "SELECT "
			"chart_path, song_dir, section_dir, section_display_name, skip_section_heading,"
			"visible_in_directory_name_sort, visible_in_all_name_sort, section_directory_index, song_directory_index, chart_file_index, song_sort_key,"
			"difficulty_idx, level,"
			"title, title_translit, title_img_filename, artist, artist_translit, artist_img_filename, chart_author,"
			"disp_bpm, std_bpm, jacket_filename, jacket_author, icon_filename, information,"
			"bgm_filename, bgm_vol, preview_offset, preview_duration, folder_volume_scale, folder_offset_ms, folder_tweet_option"
			" FROM charts ORDER BY section_directory_index, song_directory_index, chart_file_index";
		SQLite::Statement query(db, sql);
		while (query.executeStep())
		{
			int32 col = 0;
			const FilePath chartPath = FileSystem::PathAppend(baseDirectoryPath, Unicode::FromUTF8(query.getColumn(col++).getString()));
			const FilePath songDirRel = Unicode::FromUTF8(query.getColumn(col++).getString());
			const FilePath sectionDirRel = Unicode::FromUTF8(query.getColumn(col++).getString());
			const SQLite::Column sectionDisplayNameColumn = query.getColumn(col++);
			Optional<String> sectionDisplayName = sectionDisplayNameColumn.isNull()
				? Optional<String>{}
				: Optional<String>{ Unicode::FromUTF8(sectionDisplayNameColumn.getString()) };
			const bool skipSectionHeading = query.getColumn(col++).getInt() != 0;
			const bool visibleInDirectoryNameSort = query.getColumn(col++).getInt() != 0;
			const bool visibleInAllNameSort = query.getColumn(col++).getInt() != 0;
			const int32 sectionDirectoryIndex = query.getColumn(col++).getInt();
			const int32 songDirectoryIndex = query.getColumn(col++).getInt();
			const int32 chartFileIndex = query.getColumn(col++).getInt();
			String songSortKey = Unicode::FromUTF8(query.getColumn(col++).getString());

			kson::MetaChartData chartData;
			auto& meta = chartData.meta;
			meta.difficulty.idx = query.getColumn(col++).getInt();
			meta.level = query.getColumn(col++).getInt();
			meta.title = query.getColumn(col++).getString();
			meta.titleTranslit = query.getColumn(col++).getString();
			meta.titleImgFilename = query.getColumn(col++).getString();
			meta.artist = query.getColumn(col++).getString();
			meta.artistTranslit = query.getColumn(col++).getString();
			meta.artistImgFilename = query.getColumn(col++).getString();
			meta.chartAuthor = query.getColumn(col++).getString();
			meta.dispBPM = query.getColumn(col++).getString();
			meta.stdBPM = query.getColumn(col++).getDouble();
			meta.jacketFilename = query.getColumn(col++).getString();
			meta.jacketAuthor = query.getColumn(col++).getString();
			meta.iconFilename = query.getColumn(col++).getString();
			meta.information = query.getColumn(col++).getString();
			chartData.audio.bgm.filename = query.getColumn(col++).getString();
			chartData.audio.bgm.vol = query.getColumn(col++).getDouble();
			chartData.audio.bgm.preview.offset = query.getColumn(col++).getInt();
			chartData.audio.bgm.preview.duration = query.getColumn(col++).getInt();

			FolderConfIni folderConfIni;
			folderConfIni.volumeScale = query.getColumn(col++).getDouble();
			folderConfIni.offsetMs = query.getColumn(col++).getInt();
			folderConfIni.tweetOption = Unicode::FromUTF8(query.getColumn(col++).getString());

			const int32 difficultyIdx = meta.difficulty.idx;
			rows.push_back(ChartRowResult{
				.chartInfo = std::make_shared<SelectChartInfo>(chartPath, std::move(chartData), std::move(folderConfIni)),
				.difficultyIdx = difficultyIdx,
				.songSortKey = std::move(songSortKey),
				.songDirectoryPath = FileSystem::PathAppend(baseDirectoryPath, songDirRel),
				.sectionDirectoryPath = sectionDirRel.isEmpty() ? FilePath{} : FileSystem::PathAppend(baseDirectoryPath, sectionDirRel),
				.sectionDisplayName = std::move(sectionDisplayName),
				.skipSectionHeading = skipSectionHeading,
				.visibleInDirectoryNameSort = visibleInDirectoryNameSort,
				.visibleInAllNameSort = visibleInAllNameSort,
				.sectionDirectoryIndex = sectionDirectoryIndex,
				.songDirectoryIndex = songDirectoryIndex,
				.chartFileIndex = chartFileIndex,
			});
		}

		return rows;
	}

	Array<ChartRowResult> LoadDirectoryRows(FilePathView directoryPath, bool forceRebuild, bool checkFileChanges, StringView fnName)
	{
		const FilePath baseDirectoryPath = FileSystem::FullPath(directoryPath);
		const FilePath dbPath = DbPath(baseDirectoryPath);

		const auto loadOnce = [&](bool rebuild) -> Array<ChartRowResult>
		{
			Array<SourceFileInfo> sourceFiles = CollectSourceFiles(baseDirectoryPath);
			FileSystem::CreateDirectories(FileSystem::ParentPath(dbPath));
			SQLite::Database db(dbPath.toUTF8(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
			if (rebuild || !IsCacheValid(db, baseDirectoryPath, sourceFiles))
			{
				Rebuild(db, baseDirectoryPath, sourceFiles);
			}
			Array<ChartRowResult> rows = ReadChartRows(db, baseDirectoryPath);
			s_memoryCache[baseDirectoryPath] = MemoryCacheEntry{ .sourceFiles = std::move(sourceFiles), .rows = rows };
			return rows;
		};

		try
		{
			if (!forceRebuild)
			{
				if (const auto it = s_memoryCache.find(baseDirectoryPath); it != s_memoryCache.end())
				{
					// ファイルの変更を確認しない場合は、読み込み済みの内容をそのまま再利用する
					if (!checkFileChanges || CollectSourceFiles(baseDirectoryPath) == it->second.sourceFiles)
					{
						return it->second.rows;
					}
				}
			}
			return loadOnce(forceRebuild);
		}
		catch (const std::exception& e)
		{
			Logger << U"[ksm warning] SelectMenuCacheDb::{} failed. Fallback to rebuild. (path:'{}', error:'{}')"_fmt(fnName, directoryPath, Unicode::FromUTF8(e.what()));
			FileSystem::Remove(dbPath);
			try
			{
				return loadOnce(true);
			}
			catch (const std::exception& e2)
			{
				Logger << U"[ksm warning] SelectMenuCacheDb::{} rebuild failed. (path:'{}', error:'{}')"_fmt(fnName, directoryPath, Unicode::FromUTF8(e2.what()));
				return {};
			}
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

Array<SelectCachedSongEntry> SelectMenuCacheDb::LoadDirectoryForNameSort(FilePathView directoryPath, bool forceRebuild, bool checkFileChanges)
{
	Array<ChartRowResult> rows = LoadDirectoryRows(directoryPath, forceRebuild, checkFileChanges, U"LoadDirectoryForNameSort");
	rows.remove_if([](const ChartRowResult& row) { return !row.visibleInDirectoryNameSort; });

	// 楽曲フォルダ直下の譜面(セクションなし)を先頭に、以降はセクション・楽曲フォルダ・譜面ファイルの順
	std::stable_sort(rows.begin(), rows.end(), [](const ChartRowResult& a, const ChartRowResult& b)
	{
		return (a.sectionDirectoryIndex < 0 ? 0 : 1) < (b.sectionDirectoryIndex < 0 ? 0 : 1);
	});
	return GroupRowsAsSongEntries(rows);
}

Array<SelectCachedSongEntry> SelectMenuCacheDb::LoadDirectoryForAllNameSort(FilePathView directoryPath, bool forceRebuild, bool checkFileChanges)
{
	Array<ChartRowResult> rows = LoadDirectoryRows(directoryPath, forceRebuild, checkFileChanges, U"LoadDirectoryForAllNameSort");
	rows.remove_if([](const ChartRowResult& row) { return !row.visibleInAllNameSort; });
	std::stable_sort(rows.begin(), rows.end(), [](const ChartRowResult& a, const ChartRowResult& b)
	{
		return a.songSortKey < b.songSortKey;
	});
	return GroupRowsAsSongEntries(rows);
}

Array<SelectCachedChartEntry> SelectMenuCacheDb::LoadDirectoryForLevelSort(FilePathView directoryPath, bool forceRebuild, bool checkFileChanges)
{
	return RowsToChartEntries(LoadDirectoryRows(directoryPath, forceRebuild, checkFileChanges, U"LoadDirectoryForLevelSort"));
}

void SelectMenuCacheDb::Invalidate(FilePathView directoryPath)
{
	if (!directoryPath.empty() && FileSystem::IsDirectory(directoryPath))
	{
		const FilePath baseDirectoryPath = FileSystem::FullPath(directoryPath);
		s_memoryCache.erase(baseDirectoryPath);
		FileSystem::Remove(DbPath(baseDirectoryPath));
	}
}

void SelectMenuCacheDb::InvalidateAll()
{
	for (const auto& directory : GetSubDirectories(FsUtils::SongsDirectoryPath()))
	{
		Invalidate(directory);
	}
}
