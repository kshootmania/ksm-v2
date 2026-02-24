#!/usr/bin/env python3
"""
K-Shoot MANIA v2 Windows配布用zip作成スクリプト

files_win.tsvに記載されたファイル一覧をもとに配布用zipを作成する。
"""

import re
import shutil
import zipfile
from pathlib import Path


def get_project_root():
	"""プロジェクトルートディレクトリを取得"""
	script_dir = Path(__file__).resolve().parent
	return script_dir.parent


def get_version_from_cmake():
	"""CMakeLists.txtからバージョン文字列を取得"""
	project_root = get_project_root()
	cmake_path = project_root / "CMakeLists.txt"

	if not cmake_path.exists():
		print(f"警告: {cmake_path} が見つかりません")
		return "unknown"

	with open(cmake_path, "r", encoding="utf-8") as f:
		content = f.read()

	version_match = re.search(r'project\(\s*kshootmania\s+VERSION\s+(\S+)\s*\)', content)
	suffix_match = re.search(r'set\(\s*PROJECT_VERSION_SUFFIX\s+"([^"]*)"\s*\)', content)

	if version_match:
		version = version_match.group(1)
		suffix = suffix_match.group(1) if suffix_match else ""
		return version + suffix

	print("警告: CMakeLists.txtからバージョンを取得できませんでした")
	return "unknown"


def load_file_mappings(tsv_path):
	"""TSVファイルからファイルマッピングを読み込む"""
	mappings = []
	with open(tsv_path, "r", encoding="utf-8") as f:
		for line in f:
			line = line.strip()
			if not line or line.startswith("#"):
				continue

			parts = line.split("\t")
			if len(parts) == 2:
				source, dest = parts
				mappings.append((source, dest))

	return mappings


def copy_files(file_mappings, output_dir, version):
	"""ファイルマッピングに従ってファイルをコピー"""
	project_root = get_project_root()

	if output_dir.exists():
		print(f"既存の出力ディレクトリを削除中: {output_dir}")
		shutil.rmtree(output_dir)

	output_dir.mkdir(parents=True, exist_ok=True)

	copied_count = 0
	total_count = len(file_mappings)
	missing_files = []

	print(f"\nファイルをコピー中 (合計 {total_count} ファイル)...")

	for source_rel, dest_rel in file_mappings:
		source_path = project_root / source_rel
		dest_path = output_dir / dest_rel

		if not source_path.exists():
			print(f"  警告: ファイルが見つかりません: {source_path}")
			missing_files.append(source_rel)
			continue

		dest_path.parent.mkdir(parents=True, exist_ok=True)

		if dest_rel == "readme.txt":
			with open(source_path, "r", encoding="utf-8") as f:
				content = f.read()
			content = content.replace("@@VERSION@@", version)
			with open(dest_path, "w", encoding="utf-8") as f:
				f.write(content)
			print(f"  readme.txt: @@VERSION@@ を {version} に置換")
		else:
			shutil.copy2(source_path, dest_path)

		copied_count += 1

		if copied_count % 50 == 0:
			print(f"  {copied_count} / {total_count} ファイルをコピー...")

	print(f"  {copied_count} / {total_count} ファイルをコピー完了")

	if missing_files:
		print(f"\nエラー: {len(missing_files)} ファイルが見つかりませんでした:")
		for f in missing_files[:20]:
			print(f"  - {f}")
		if len(missing_files) > 20:
			print(f"  ... 他 {len(missing_files) - 20} ファイル")
		raise FileNotFoundError(f"{len(missing_files)} ファイルが見つかりません。処理を中断します。")

	return copied_count


def create_zip(output_dir, zip_path):
	"""zipファイルを作成"""
	print(f"\nzipファイルを作成中: {zip_path}")

	if zip_path.exists():
		zip_path.unlink()

	with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zf:
		for file_path in output_dir.rglob("*"):
			if file_path.is_file():
				arcname = file_path.relative_to(output_dir.parent)
				zf.write(file_path, arcname)

	zip_size_mb = zip_path.stat().st_size / (1024 * 1024)
	print(f"zip作成完了: {zip_path.name} (サイズ: {zip_size_mb:.2f} MB)")


def show_zip_contents(zip_path, max_lines=20):
	"""zipファイルの内容を表示"""
	print(f"\n=== 作成されたzipの内容 (先頭{max_lines}ファイル) ===")

	with zipfile.ZipFile(zip_path, "r") as zf:
		file_list = zf.namelist()
		for i, name in enumerate(sorted(file_list)[:max_lines], 1):
			file_info = zf.getinfo(name)
			size_kb = file_info.file_size / 1024
			print(f"  {i:3d}. {name} ({size_kb:.1f} KB)")

		if len(file_list) > max_lines:
			print(f"  ... 他 {len(file_list) - max_lines} ファイル")

		print(f"\n合計: {len(file_list)} ファイル")


def main():
	print("=== K-Shoot MANIA v2 Windows配布用zip作成 ===")

	script_dir = Path(__file__).resolve().parent
	tsv_path = script_dir / "files_win.tsv"
	output_dir = script_dir / "kshootmania_v2"
	zip_path = script_dir / "kshootmania_v2_win.zip"

	try:
		version = get_version_from_cmake()
		print(f"バージョン: {version}")

		print(f"TSVファイル: {tsv_path}")
		print(f"出力ディレクトリ: {output_dir}")
		print(f"zip出力先: {zip_path}")

		if not tsv_path.exists():
			print(f"\nエラー: {tsv_path} が見つかりません")
			return 1

		print(f"\n{tsv_path.name} からファイル一覧を読み込み中...")
		file_mappings = load_file_mappings(tsv_path)
		print(f"  {len(file_mappings)} ファイルを読み込み")

		copied_count = copy_files(file_mappings, output_dir, version)
		create_zip(output_dir, zip_path)
		show_zip_contents(zip_path)

		print("\n=== 完了 ===")
		print(f"出力先: {zip_path}")
		return 0

	except FileNotFoundError as e:
		print(f"\nエラー: {e}")
		print("\n処理を中断しました。")
		if output_dir.exists():
			print(f"出力ディレクトリを削除中: {output_dir}")
			shutil.rmtree(output_dir)
		return 1
	except Exception as e:
		print(f"\n予期しないエラーが発生しました: {e}")
		if output_dir.exists():
			print(f"出力ディレクトリを削除中: {output_dir}")
			shutil.rmtree(output_dir)
		return 1


if __name__ == "__main__":
	import sys
	sys.exit(main())
