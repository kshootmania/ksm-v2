#include "SimpleDialog.hpp"
#include "Input/KeyConfig.hpp"
#include "Input/InputUtils.hpp"
#include "Common/FsUtils.hpp"
#include "Graphics/ScreenUtils.hpp"

SimpleDialog::SimpleDialog(String titleText, String bodyText)
	: m_titleText(std::move(titleText))
	, m_bodyText(std::move(bodyText))
{
}

Co::Task<void> SimpleDialog::start()
{
	// Canvasをロード
	const FilePath uiFilePath = FsUtils::GetResourcePath(U"ui/dialog/simple_dialog.noco");
	m_canvas = noco::Canvas::LoadFromFile(uiFilePath);
	if (!m_canvas)
	{
		co_return;
	}

	// パラメータ設定
	m_canvas->setParamValue(U"titleText", m_titleText);
	m_canvas->setParamValue(U"bodyText", m_bodyText);
	m_canvas->setParamValue(U"hasTitle", !m_titleText.isEmpty());

	// ヒットテスト座標を描画と一致させるため中央寄せとスケーリングをCanvas側に設定
	m_canvas->setAutoFitMode(noco::AutoFitMode::None);

	// Start・Back・クリックのいずれかで閉じる
	while (true)
	{
		const double scale = ScreenUtils::Scaled(1.0);
		const Vec2 position = (Scene::Size() - m_canvas->referenceSize() * scale) / 2.0;
		m_canvas->setPositionScale(position, Vec2{ scale, scale });

		// 画面全体がクリック可能なのでカーソルを人差し指に変更
		Cursor::RequestStyle(CursorStyle::Hand);

		m_canvas->update(noco::HitTestEnabledYN{ InputUtils::IsUIMouseInputEnabled() });

		// ウィンドウがアクティブ化されたフレームのクリックは入力として扱わない
		if (KeyConfig::Down(kButtonStart) || KeyConfig::Down(kButtonBack) || (MouseL.down() && !InputUtils::WindowFocusedThisFrame()))
		{
			break;
		}
		co_await Co::NextFrame();
	}
}

void SimpleDialog::draw() const
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
