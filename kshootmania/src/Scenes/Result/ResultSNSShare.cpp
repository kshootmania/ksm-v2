#include "ResultSNSShare.hpp"
#include "Input/KeyConfig.hpp"

ResultSNSShare::ResultSNSShare(StringView tweetText)
	: m_tweetText{ tweetText }
{
}

void ResultSNSShare::update(noco::Canvas* pCanvas)
{
	if (m_screenshotRequested && ScreenCapture::HasNewFrame())
	{
		Clipboard::SetImage(ScreenCapture::GetFrame());
		m_screenshotRequested = false;

		// スクリーンショット用の一時的な変更を元に戻す
		pCanvas->setParamValue(U"bottomRightText", U"");
		if (const auto snsButton = pCanvas->findByName(U"SNSButton"))
		{
			snsButton->setActive(true);
		}

		System::MessageBoxOK(I18n::Get(I18n::Result::ScreenshotCopied));

		const String encodedText = PercentEncode(m_tweetText);
		System::LaunchBrowser(U"https://x.com/intent/tweet?text=" + encodedText);
	}

	if (pCanvas->isEventFiredWithTag(U"onClickSNSButton") || KeyConfig::Down(kButtonAutoPlay) || (KeyShift.pressed() && KeyConfig::Down(kButtonStart)))
	{
		// スクリーンショット用に一時的にバージョン番号表示、SNSボタン非表示
		pCanvas->setParamValue(U"bottomRightText", String{ kAppVersion });
		if (const auto snsButton = pCanvas->findByName(U"SNSButton"))
		{
			snsButton->setActive(false);
		}
		pCanvas->update(); // 表示反映のため再更新

		ScreenCapture::RequestCurrentFrame();
		m_screenshotRequested = true;
	}
}
