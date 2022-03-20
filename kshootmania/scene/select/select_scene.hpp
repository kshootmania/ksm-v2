#pragma once

class SelectScene : public SceneManager<StringView>::Scene
{
private:
	const Texture m_bgTexture;

public:
	explicit SelectScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;
};
