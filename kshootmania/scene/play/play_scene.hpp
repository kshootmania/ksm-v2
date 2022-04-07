#pragma once

class PlayScene : public MyScene
{
public:
	explicit PlayScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;
};
