#include "SimpleDialog.hpp"
#include "Input/KeyConfig.hpp"
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

	// StartまたはBackで閉じる
	while (true)
	{
		m_canvas->update(m_canvas->referenceSize());

		if (KeyConfig::Down(kButtonStart) || KeyConfig::Down(kButtonBack))
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

	// ダイアログを画面中央にスケーリング描画
	const double scale = ScreenUtils::Scaled(1.0);
	const Vec2 dialogSize = m_canvas->referenceSize() * scale;
	const Vec2 pos = (Scene::Size() - dialogSize) / 2.0;
	const Transformer2D dialogTransform{ Mat3x2::Scale(scale).translated(pos) };
	m_canvas->draw();
}
