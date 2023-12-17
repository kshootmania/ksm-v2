#include "screen_fade_addon.hpp"

namespace
{
	ScreenFadeAddon& GetInstance()
	{
		ScreenFadeAddon* const pAddon = Addon::GetAddon<ScreenFadeAddon>(U"ScreenFade");
		if (pAddon == nullptr)
		{
			throw Error(U"ScreenFade addon is not registered");
		}
		return *pAddon;
	}
}

void ScreenFadeAddon::fadeImpl(State fadeState, const Duration& duration, std::function<void()> callback, const Color& color)
{
	if (m_state != State::None && color == m_color)
	{
		// 既にフェード中かつ同じ色であれば、アルファ値を引き継ぐ
		// (フェードイン中にフェードアウトを呼び出した場合に不自然にならないようにするため)
		// TODO: フェードインとフェードアウトを別々のインスタンスにして重ねれば異なる色でも不自然にならなくなる
		m_startAlpha = fadeAlpha();
	}
	else
	{
		m_startAlpha = 0.0;
	}

	m_state = fadeState;
	m_callback = std::move(callback);
	m_color = color;
	m_timer.set(duration);
	m_isTimerStarted = false;
}

double ScreenFadeAddon::fadeAlpha() const
{
	if (m_state == State::None)
	{
		return 0.0;
	}

	double targetAlpha;
	switch (m_state)
	{
	case State::FadeIn:
		targetAlpha = 0.0;
		break;
	case State::FadeOut:
		targetAlpha = 1.0;
		break;
	default:
		throw Error(U"Invalid state");
	}

	return std::lerp(m_startAlpha, targetAlpha, m_timer.progress0_1());
}

bool ScreenFadeAddon::update()
{
	if (m_state == State::None)
	{
		return true;
	}

	if (m_timer.reachedZero())
	{
		if (m_callback)
		{
			m_callback();
			m_callback = nullptr;
		}
		if (m_state == State::FadeIn)
		{
			// フェードインが終わったらNoneにする
			// (フェードアウトの場合はフェードインされるまでそのままなのでNoneにしない)
			m_state = State::None;
			m_isTimerStarted = false;
			return true;
		}
	}

	// タイマーを開始
	// (fadeImplの呼び出しから次フレームまでに時間がかかる場合も考えられるので、update内で開始する)
	if (m_state != State::None && !m_isTimerStarted)
	{
		m_timer.restart();
		m_isTimerStarted = true;
	}

	return true;
}

void ScreenFadeAddon::draw() const
{
	if (m_state == State::None || !m_isTimerStarted)
	{
		return;
	}

	Scene::Rect().draw(ColorF{ m_color }.withAlpha(fadeAlpha()));
}

void ScreenFadeAddon::FadeIn(const Duration& fadeTimeSec, std::function<void()> callback, const Color& color)
{
	::GetInstance().fadeImpl(State::FadeIn, fadeTimeSec, std::move(callback), color);
}

void ScreenFadeAddon::FadeIn(std::function<void()> callback, const Color& color)
{
	::GetInstance().fadeImpl(State::FadeIn, kDefaultDuration, std::move(callback), color);
}

void ScreenFadeAddon::FadeOut(const Duration& fadeTimeSec, std::function<void()> callback, const Color& color)
{
	::GetInstance().fadeImpl(State::FadeOut, fadeTimeSec, std::move(callback), color);
}

void ScreenFadeAddon::FadeOut(std::function<void()> callback, const Color& color)
{
	::GetInstance().fadeImpl(State::FadeOut, kDefaultDuration, std::move(callback), color);
}

void ScreenFadeAddon::FadeOutToScene(const String& sceneName, const Duration& duration, const Color& color)
{
	const auto callback = [sceneNameCopy = sceneName]() // 呼び出しタイミングが後なのでsceneNameは要コピーキャプチャ
	{
		SceneManagerAddon::ChangeScene(sceneNameCopy);
	};
	::GetInstance().fadeImpl(State::FadeOut, duration, callback, color);
}

bool ScreenFadeAddon::IsFading()
{
	return ::GetInstance().m_state != State::None;
}

bool ScreenFadeAddon::IsFadingIn()
{
	return ::GetInstance().m_state == State::FadeIn;
}

bool ScreenFadeAddon::IsFadingOut()
{
	return ::GetInstance().m_state == State::FadeOut;
}
