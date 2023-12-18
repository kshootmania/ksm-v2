#pragma once

/// @brief 画面フェード
class ScreenFade
{
private:
	/// @brief 開始アルファ値
	const double m_startAlpha;

	/// @brief 目標アルファ値
	const double m_targetAlpha;

	/// @brief フェード色
	Color m_color = Palette::Black;

	/// @brief タイマー
	Timer m_timer;

	/// @brief フェードが完了済みか
	bool m_isFinished = false;

	/// @brief フェード終了時に実行するコールバック関数
	std::function<void()> m_callback = nullptr;

	/// @brief 現在のアルファ値を取得
	double currentAlpha() const;

public:
	/// @brief コンストラクタ
	/// @param startAlpha 開始アルファ値
	/// @param targetAlpha 目標アルファ値
	explicit ScreenFade(double startAlpha, double targetAlpha);

	/// @brief フェード開始
	/// @param duration フェード時間
	/// @param color フェード色
	/// @param callback フェード終了時に実行するコールバック関数
	void start(const Duration& duration, const Color& color, std::function<void()> callback = nullptr);

	/// @brief フェード更新
	/// @return フェードが完了したか
	void update();

	/// @brief 描画
	void draw() const;

	/// @brief フェードを一時停止
	void pause();

	/// @brief フェードを終了してリセット
	void reset();

	/// @brief フェードが開始済みか
	/// @return フェードが開始済みならtrue
	bool isStarted() const;

	/// @brief フェード中か(一時停止中も含む)
	/// @return フェード中ならtrue
	bool isRunning() const;

	/// @brief フェードが完了済みか
	/// @return フェードが完了済みならtrue
	bool isFinished() const;
};

/// @brief 画面フェードを実行するアドオン
/// @note 元々はOpenSiv3DのSceneManagerの画面フェードを使用していたが、
///		  ・フェードアウトとフェードインに別々の時間の長さを指定できない
/// 	  ・シーンのコンストラクタで処理に時間がかかると前のシーンが薄く見えた状態が続いてしまう
///       ・フェード中に一部処理のみ実行しないようにするためにはupdateImpl等の共通関数を用意してフェード中かを引数で渡す必要がありやや面倒
///       といった問題が生じたためアドオンとして自前実装
class ScreenFadeAddon : public IAddon
{
private:
	ScreenFade m_fadeIn{ 1.0, 0.0 };

	ScreenFade m_fadeOut{ 0.0, 1.0 };

public:
	static constexpr Duration kDefaultDuration = 0.6s;
	static constexpr double kDefaultDurationSec = kDefaultDuration.count();

	ScreenFadeAddon() = default;

	virtual bool update() override;

	virtual void draw() const override;

	static void FadeIn(const Duration& duration = kDefaultDuration, const Color& color = Palette::Black);

	static void FadeIn(const Color& color);

	static void FadeOut(const Duration& duration = kDefaultDuration, std::function<void()> callback = nullptr, const Color& color = Palette::Black);

	static void FadeOut(std::function<void()> callback, const Color& color = Palette::Black);

	static void FadeOutToScene(const String& sceneName, const Duration& duration = kDefaultDuration, const Color& color = Palette::Black);

	static bool IsFading();

	static bool IsFadingIn();

	static bool IsFadingOut();
};
