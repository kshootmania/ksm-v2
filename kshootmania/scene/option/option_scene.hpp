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
	Texture m_bgTexture;

	TiledTexture m_headerTiledTexture;

	OptionTopMenu m_topMenu;

	std::array<OptionMenu, kOptionMenuTypeEnumCount> m_optionMenus;

	Font m_font;

	Optional<OptionMenuType> m_currentOptionMenuIdx = none;

public:
	static constexpr StringView kSceneName = U"Option";

	explicit OptionScene(const InitData& initData);

	virtual void update() override;

	virtual void draw() const override;

	void exitScene();
};
