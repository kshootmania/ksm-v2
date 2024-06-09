#include "show_loading_one_frame.hpp"

ShowLoadingOneFrame::ShowLoadingOneFrame(HasBgYN hasBg)
	: m_hasBg(hasBg)
{
}

Co::Task<void> ShowLoadingOneFrame::start()
{
	co_await Co::NextFrame();
}

void ShowLoadingOneFrame::draw() const
{
	if (m_hasBg)
	{
		FitToHeight(m_bgTexture).drawAt(Scene::Center());
	}
	else
	{
		const Transformer2D transform{ Mat3x2::Identity(), Transformer2D::Target::SetLocal };
		Scene::Rect().draw(Palette::Black);
	}

	m_loadingTexture.resized(Scaled(200, 30)).draw(Scene::Width() - Scaled(208), Scaled(442));
}

Co::Task<void> ShowLoadingOneFrame::Play(HasBgYN hasBg)
{
	return Co::Play<ShowLoadingOneFrame>(hasBg);
}
