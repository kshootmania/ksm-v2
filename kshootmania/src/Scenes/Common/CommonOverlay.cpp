#include "CommonOverlay.hpp"
#include "Input/KeyConfig.hpp"

namespace
{
	std::shared_ptr<noco::Canvas> LoadCommonOverlayCanvas()
	{
		const FilePath uiFilePath = FsUtils::GetResourcePath(U"ui/scene/common_overlay.noco");
		const auto canvas = noco::Canvas::LoadFromFile(uiFilePath);
		if (!canvas)
		{
			throw Error{ U"Failed to load '{}'"_fmt(uiFilePath) };
		}
		return canvas;
	}
}

CommonOverlay::CommonOverlay()
	: m_canvas(LoadCommonOverlayCanvas())
{
}

CommonOverlay::CommonOverlay(const Duration& backButtonHoldDuration)
	: m_canvas(LoadCommonOverlayCanvas())
	, m_backButtonHoldDuration(backButtonHoldDuration)
{
}

void CommonOverlay::update(noco::HitTestEnabledYN hitTestEnabled)
{
	m_canvas->update(hitTestEnabled);

	if (m_canvas->isEventFiredWithTag(U"onPressStartBackButton"))
	{
		m_backButtonPressed = true;
	}
	if (m_canvas->isEventFiredWithTag(U"onPressEndBackButton"))
	{
		m_backButtonPressed = false;
	}

	if (m_backButtonHoldDuration.has_value())
	{
		// 長押しモードでは押下中にゲージが進行し、完了した場合のみBack扱いにする
		if (m_backButtonPressed && m_backButtonProgress < 1.0)
		{
			const double holdDurationSec = m_backButtonHoldDuration->count();
			m_backButtonHoldSec += Scene::DeltaTime();
			m_backButtonProgress = holdDurationSec <= 0.0 ? 1.0 : Min(m_backButtonHoldSec / holdDurationSec, 1.0);
			if (m_backButtonProgress >= 1.0)
			{
				KeyConfig::RequestVirtualDown(kButtonBack);
			}
		}
		else if (!m_backButtonPressed)
		{
			// 途中で離した場合はゲージを戻す
			m_backButtonHoldSec = 0.0;
			m_backButtonProgress = 0.0;
		}
	}
	else
	{
		// クリックモードではゲージ表示は使用しない
		m_backButtonProgress = 0.0;

		// Backボタンのクリックをボタン入力と同じ効果にする
		if (m_canvas->isEventFiredWithTag(U"onClickBackButton"))
		{
			KeyConfig::RequestVirtualDown(kButtonBack);
		}
	}

	m_canvas->setParamValue(U"backButtonProgress", m_backButtonProgress);
}

void CommonOverlay::draw() const
{
	m_canvas->draw();
}

void CommonOverlay::setBackButtonVisible(bool visible)
{
	m_canvas->setParamValue(U"backButtonVisible", visible);
}
