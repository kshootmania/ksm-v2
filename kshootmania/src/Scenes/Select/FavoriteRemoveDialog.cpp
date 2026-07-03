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

	// ヒットテスト座標を描画と一致させるため中央寄せとスケーリングをCanvas側に設定
	m_canvas->setAutoFitMode(noco::AutoFitMode::None);

	// 選択確定までループ
	while (true)
	{
		m_menu.update();

		// クリックされた選択肢がカーソル位置と同じ場合は決定、異なる場合はカーソル移動
		Optional<int32> clickedChoice;
		if (m_canvas->isEventFiredWithTag(U"onClickYes"))
		{
			clickedChoice = static_cast<int32>(FavoriteRemoveChoice::Yes);
		}
		else if (m_canvas->isEventFiredWithTag(U"onClickNo"))
		{
			clickedChoice = static_cast<int32>(FavoriteRemoveChoice::No);
		}
		bool clickDecided = false;
		if (clickedChoice.has_value())
		{
			if (*clickedChoice == m_menu.cursor())
			{
				clickDecided = true;
			}
			else
			{
				m_menu.setCursor(*clickedChoice);
			}
		}

		// Canvasパラメータに反映
		m_canvas->setParamValue(U"cursorIndex", m_menu.cursor());

		const double scale = ScreenUtils::Scaled(1.0);
		const Vec2 position = (Scene::Size() - m_canvas->referenceSize() * scale) / 2.0;
		m_canvas->setPositionScale(position, Vec2{ scale, scale });

		m_canvas->update();

		if (KeyConfig::Down(kButtonStart) || clickDecided)
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

	m_canvas->draw();
}
