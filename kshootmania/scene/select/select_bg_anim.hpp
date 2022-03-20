#pragma once

class SelectBGAnim
{
private:
	const Texture m_texture;

	Stopwatch m_stopwatch;

public:
	SelectBGAnim();

	void draw() const;
};
