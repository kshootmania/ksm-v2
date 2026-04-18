#include "FavoriteAddDialog.hpp"
#include "Common/FsUtils.hpp"
#include "Common/Encoding.hpp"
#include "Input/Cursor/CursorInput.hpp"
#include "Input/KeyConfig.hpp"
#include "Graphics/ScreenUtils.hpp"

namespace
{
	int32 CalculateMaxSelectableNumber()
	{
		const FilePath songsDir = FsUtils::SongsDirectoryPath();

		for (int32 i = 1; i <= 9; ++i)
		{
			const FilePath favPath = FileSystem::PathAppend(songsDir, U"Favorite{}.fav"_fmt(i));
			if (!FileSystem::Exists(favPath))
			{
				return i;
			}
		}

		return 9;
	}
}

FavoriteAddDialog::FavoriteAddDialog()
	: m_menu(LinearMenu::CreateInfoWithCursorMinMax{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Vertical,
			.buttonFlags = CursorButtonFlags::kArrowOrBT,
		},
		.cursorMin = 1,
		.cursorMax = 1,
		.cyclic = IsCyclicMenuYN::Yes,
		.defaultCursor = 1,
	})
{
}

Co::Task<Optional<int32>> FavoriteAddDialog::start()
{
	// Canvasをロード
	const FilePath uiFilePath = FsUtils::GetResourcePath(U"ui/dialog/favorite_add_dialog.noco");
	m_canvas = noco::Canvas::LoadFromFile(uiFilePath);
	if (!m_canvas)
	{
		co_return none;
	}

	// 最大選択可能番号を算出してメニュー設定
	const int32 maxNumber = CalculateMaxSelectableNumber();
	m_menu.setCursorMax(maxNumber);
	m_menu.setCursor(1);

	// 選択確定までループ
	while (true)
	{
		m_menu.update();

		// Canvasパラメータに反映(UIは0始まりなので-1)
		m_canvas->setParamValue(U"numberIndex", m_menu.cursor() - 1);

		m_canvas->update(m_canvas->referenceSize());

		if (KeyConfig::Down(kButtonStart))
		{
			co_return m_menu.cursor();
		}

		if (KeyConfig::Down(kButtonBack))
		{
			co_return none;
		}

		co_await Co::NextFrame();
	}
}

void FavoriteAddDialog::draw() const
{
	if (!m_canvas)
	{
		return;
	}

	const Transformer2D resetTransform{ Mat3x2::Identity(), Transformer2D::Target::SetLocal };

	// 半透明オーバーレイ
	Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.5 });

	// ダイアログを画面中央にスケーリング描画
	const double scale = ScreenUtils::Scaled(1.0);
	const Vec2 dialogSize = m_canvas->referenceSize() * scale;
	const Vec2 pos = (Scene::Size() - dialogSize) / 2.0;
	const Transformer2D dialogTransform{ Mat3x2::Scale(scale).translated(pos) };
	m_canvas->draw();
}

FilePath GetFavoriteFilePath(int32 favoriteNumber)
{
	const FilePath songsDir = FsUtils::SongsDirectoryPath();
	return FileSystem::PathAppend(songsDir, U"Favorite{}.fav"_fmt(favoriteNumber));
}

bool AddSongToFavorite(int32 favoriteNumber, StringView songPath)
{
	const FilePath favPath = GetFavoriteFilePath(favoriteNumber);

	// パス区切りは/に統一
	String normalizedPath{ songPath };
	normalizedPath.replace(U"\\", U"/");

	// 末尾のスラッシュを削除
	if (normalizedPath.ends_with(U'/'))
	{
		normalizedPath.pop_back();
	}

	// 既存の内容を読み込み
	Array<String> lines;
	if (FileSystem::Exists(favPath))
	{
		lines = Encoding::ReadTextFileLinesShiftJISOrUTF8BasedOnBOM(favPath);
		for (auto& line : lines)
		{
			line = line.trimmed();
			line.replace(U"\\", U"/");
			if (line.ends_with(U'/'))
			{
				line.pop_back();
			}
		}
		lines.remove_if([](const String& s) { return s.isEmpty(); });
	}

	// 重複チェック
	if (lines.contains(normalizedPath))
	{
		return false;
	}

	lines.push_back(normalizedPath);

	// UTF-8 BOM付きで保存
	TextWriter writer(favPath, TextEncoding::UTF8_WITH_BOM);
	for (const auto& line : lines)
	{
		writer.writeln(line);
	}

	return true;
}
