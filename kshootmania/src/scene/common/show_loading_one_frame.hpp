#pragma once
#include "common_assets.hpp"

using HasBgYN = YesNo<struct HasBg_tag>;

class ShowLoadingOneFrame : public Co::SequenceBase<void>
{
private:
	const HasBgYN m_hasBg;
	const Texture m_bgTexture{ TextureAsset(CommonTexture::kMainBG) };
	const Texture m_loadingTexture{ TextureAsset(CommonTexture::kLoading) };

	Co::Task<void> start() override;

	void draw() const override;

public:
	explicit ShowLoadingOneFrame(HasBgYN hasBg);

	static Co::Task<void> Play(HasBgYN hasBg);
};
