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

double ScreenFade::currentAlpha() const
{
	if (isRunning())
	{
		return std::lerp(m_startAlpha, m_targetAlpha, m_timer.progress0_1());
	}
	else
	{
		return m_isFinished ? m_targetAlpha : 0.0;
	}
}

ScreenFade::ScreenFade(double startAlpha, double targetAlpha)
	: m_startAlpha(startAlpha)
	, m_targetAlpha(targetAlpha)
{
}

void ScreenFade::start(const Duration& duration, const Color& color, std::function<void()> callback)
{
	if (isRunning())
	{
#ifdef _DEBUG
		Print << U"ScreenFadeでフェード中に多重にフェード開始された";
#endif
		return;
	}

	m_color = color;
	m_callback = std::move(callback);
	m_timer.set(duration);
	m_timer.start();
}

void ScreenFade::update()
{
	if (!m_timer.isStarted())
	{
		return;
	}

	if (!m_isFinished && m_timer.reachedZero())
	{
		if (m_callback)
		{
			m_callback();
		}
		m_isFinished = true;
	}
}

void ScreenFade::draw() const
{
	if (!m_timer.isStarted())
	{
		return;
	}

	const double alpha = currentAlpha();
	if (alpha > 0.0)
	{
		Scene::Rect().draw(ColorF{ m_color }.withAlpha(alpha));
	}
}

void ScreenFade::pause()
{
	m_timer.pause();
}

void ScreenFade::reset()
{
	m_timer.reset();
	m_isFinished = false;
}

bool ScreenFade::isStarted() const
{
	return m_timer.isStarted();
}

bool ScreenFade::isRunning() const
{
	return m_timer.isRunning() || m_timer.isPaused();
}

bool ScreenFade::isFinished() const
{
	return m_isFinished;
}

bool ScreenFadeAddon::update()
{
	m_fadeIn.update();
	m_fadeOut.update();
	return true;
}

void ScreenFadeAddon::draw() const
{
	m_fadeIn.draw();
	m_fadeOut.draw();
}

void ScreenFadeAddon::FadeIn(const Duration& duration, const Color& color)
{
	ScreenFadeAddon& instance = ::GetInstance();
	instance.m_fadeIn.start(duration, color, nullptr);
	instance.m_fadeOut.reset();
}

void ScreenFadeAddon::FadeIn(const Color& color)
{
	ScreenFadeAddon& instance = ::GetInstance();
	instance.m_fadeIn.start(kDefaultDuration, color, nullptr);
	instance.m_fadeOut.reset();
}

void ScreenFadeAddon::FadeOut(const Duration& duration, std::function<void()> callback, const Color& color)
{
	auto callbackImpl = [callbackMoved = std::move(callback)]()
	{
		callbackMoved();
		::GetInstance().m_fadeIn.reset();
	};

	ScreenFadeAddon& instance = ::GetInstance();
	instance.m_fadeOut.start(duration, color, std::move(callbackImpl));
	instance.m_fadeIn.pause();
}

void ScreenFadeAddon::FadeOut(std::function<void()> callback, const Color& color)
{
	auto callbackImpl = [callbackMoved = std::move(callback)]()
	{
		callbackMoved();
		::GetInstance().m_fadeIn.reset();
	};

	ScreenFadeAddon& instance = ::GetInstance();
	instance.m_fadeOut.start(kDefaultDuration, color, std::move(callbackImpl));
	instance.m_fadeIn.pause();
}

void ScreenFadeAddon::FadeOutToScene(const String& sceneName, const Duration& duration, const Color& color)
{
	auto callbackImpl = [sceneNameCopy = sceneName]() // 呼び出しタイミングが後なのでsceneNameは要コピーキャプチャ
	{
		SceneManagerAddon::ChangeScene(sceneNameCopy);
		::GetInstance().m_fadeIn.reset();
	};

	ScreenFadeAddon& instance = ::GetInstance();
	instance.m_fadeOut.start(duration, color, std::move(callbackImpl));
	instance.m_fadeIn.pause();
}

bool ScreenFadeAddon::IsFading()
{
	return IsFadingIn() || IsFadingOut();
}

bool ScreenFadeAddon::IsFadingIn()
{
	ScreenFadeAddon& instance = ::GetInstance();
	return instance.m_fadeIn.isRunning();
}

bool ScreenFadeAddon::IsFadingOut()
{
	ScreenFadeAddon& instance = ::GetInstance();

	// フェードアウトの場合はタイマー終了後もアルファ値がゼロでなく、その場合もフェードアウト中とみなすためisRunning()ではなくisStarted()で判定
	return instance.m_fadeOut.isStarted();
}
