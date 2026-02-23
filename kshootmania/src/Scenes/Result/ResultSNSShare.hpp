#pragma once

class ResultSNSShare
{
private:
	String m_tweetText;
	bool m_screenshotRequested = false;

public:
	explicit ResultSNSShare(StringView tweetText);

	// Canvasからイベントを検出してスクリーンショット→ツイート処理を行う
	void update(noco::Canvas* pCanvas);
};
