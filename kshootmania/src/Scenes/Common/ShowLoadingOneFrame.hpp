#pragma once
#include "CommonAssets.hpp"

/// @brief ローディング表示時の背景の塗り方
enum class LoadingBgMode
{
	/// @brief 汎用背景付き(シーン遷移用)
	kMainBg,

	/// @brief 黒色背景付き(シーン遷移用)
	kBlack,

	/// @brief 塗りなし(重ねて表示したい場合)
	kNone,
};

class ShowLoadingOneFrame : public Co::SequenceBase<void>
{
private:
	const LoadingBgMode m_bgMode;
	const Texture m_bgTexture{ TextureAsset(CommonTexture::kMainBG) };
	const Texture m_loadingTexture{ TextureAsset(CommonTexture::kLoading) };

	Co::Task<void> start() override;

	void draw() const override;

public:
	explicit ShowLoadingOneFrame(LoadingBgMode bgMode);

	static Co::Task<void> Play(LoadingBgMode bgMode);
};
