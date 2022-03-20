#pragma once
#include "option_top_menu.hpp"
#include "option_menu.hpp"

class OptionScene : public SceneManager<StringView>::Scene
{
public:
	enum OptionMenuType : int32
	{
		kDisplaySound = 0,
		kInputJudgment,
		kOther,
		kKeyConfig,

		kOptionMenuTypeEnumCount,
	};

private:
	const Texture m_bgTexture;

	TiledTexture m_headerTiledTexture;

	OptionTopMenu m_topMenu;

	std::array<OptionMenu, kOptionMenuTypeEnumCount> m_optionMenus;

	const Font m_font;

	Optional<OptionMenuType> m_currentOptionMenuIdx = none;

public:
	explicit OptionScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;

	void exitScene();
};
