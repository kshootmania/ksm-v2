#include "FavoriteRemoveDialog.hpp"
#include "Input/Cursor/CursorInput.hpp"
#include "Input/KeyConfig.hpp"
#include "Common/FsUtils.hpp"
#include "Graphics/ScreenUtils.hpp"

FavoriteRemoveDialog::FavoriteRemoveDialog()
	: m_menu(LinearMenu::CreateInfoWithCursorMinMax{
		.cursorInputCreateInfo = {
			.type = CursorInput::Type::Horizontal,
			.buttonFlags = CursorButtonFlags::kArrowOrBT,
		},
		.cursorMin = static_cast<int32>(FavoriteRemoveChoice::No),
		.cursorMax = static_cast<int32>(FavoriteRemoveChoice::Yes),
		.cyclic = IsCyclicMenuYN::Yes,
		.defaultCursor = static_cast<int32>(FavoriteRemoveChoice::No),
	})
{
}

Co::Task<Optional<FavoriteRemoveChoice>> FavoriteRemoveDialog::start()
{
	// Canvasをロード
	const FilePath uiFilePath = FsUtils::GetResourcePath(U"ui/dialog/favorite_remove_dialog.noco");
	m_canvas = noco::Canvas::LoadFromFile(uiFilePath);
	if (!m_canvas)
	{
		co_return none;
	}

	// デフォルトはNoを選択
	m_menu.setCursor(static_cast<int32>(FavoriteRemoveChoice::No));

	// 選択確定までループ
	while (true)
	{
		m_menu.update();

		// Canvasパラメータに反映
		m_canvas->setParamValue(U"cursorIndex", m_menu.cursor());

		m_canvas->update(m_canvas->referenceSize());

		if (KeyConfig::Down(kButtonStart))
		{
			co_return static_cast<FavoriteRemoveChoice>(m_menu.cursor());
		}

		if (KeyConfig::Down(kButtonBack))
		{
			co_return none;
		}

		co_await Co::NextFrame();
	}
}

void FavoriteRemoveDialog::draw() const
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
