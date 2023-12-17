#pragma once

/// @brief 画面フェード用のアドオン
/// @note 元々はOpenSiv3DのSceneManagerの画面フェードを使用していたが、
///		  ・フェードアウトとフェードインに別々の時間の長さを指定できない
/// 	  ・シーンのコンストラクタで処理に時間がかかると前のシーンが薄く見えた状態が続いてしまう
///       ・フェード中に一部処理のみ実行しないようにするためにはupdateImpl等の共通関数を用意してフェード中かを引数で渡す必要がありやや面倒
///       といった問題が生じたためアドオンとして自前実装
class ScreenFadeAddon : public IAddon
{
private:
	enum class State
	{
		None,
		FadeIn,
		FadeOut,
	};

	State m_state = State::None;

	double m_startAlpha = 0.0;

	Color m_color = Palette::Black;

	Timer m_timer;

	bool m_isTimerStarted = false;

	std::function<void()> m_callback = nullptr;

	double fadeAlpha() const;

	void fadeImpl(State state, const Duration& duration, std::function<void()> callback, const Color& color);

public:
	static constexpr Duration kDefaultDuration = 0.6s;
	static constexpr double kDefaultDurationSec = kDefaultDuration.count();

	ScreenFadeAddon() = default;

	virtual bool update() override;

	virtual void draw() const override;

	static void FadeIn(const Duration& duration = kDefaultDuration, std::function<void()> callback = nullptr, const Color& color = Palette::Black);

	static void FadeIn(std::function<void()> callback, const Color& color = Palette::Black);

	static void FadeOut(const Duration& duration = kDefaultDuration, std::function<void()> callback = nullptr, const Color& color = Palette::Black);

	static void FadeOut(std::function<void()> callback, const Color& color = Palette::Black);

	static void FadeOutToScene(const String& sceneName, const Duration& duration = kDefaultDuration, const Color& color = Palette::Black);

	static bool IsFading();

	static bool IsFadingIn();

	static bool IsFadingOut();
};
