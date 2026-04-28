#pragma once
#include "Input/KeyConfig.hpp"
#include "MusicGame/Scroll/HispeedSetting.hpp"
#include "UI/ArrayWithLinearMenu.hpp"
#include "UI/LinearMenu.hpp"
#include <Siv3D.hpp>
#include <NocoUI.hpp>

// BTボタン押下中に表示されるオプションメニューのパネル
class BTOptionPanel
{
private:
	std::shared_ptr<noco::Canvas> m_canvas;

	// 各BTボタンメニューの項目選択用LinearMenu
	LinearMenu m_btAMenu;
	LinearMenu m_btBMenu;
	LinearMenu m_btCMenu;

	// BT-Aメニューの各項目の値変更用LinearMenu
	LinearMenu m_judgmentModeBT;
	LinearMenu m_judgmentModeFX;
	LinearMenu m_judgmentModeLaser;

	// BT-Bメニューの各項目の値変更用LinearMenu
	LinearMenu m_gaugeType;
	LinearMenu m_turnMode;
	LinearMenu m_playbackSpeed;

	// BT-Cメニューの各項目の値変更用LinearMenu
	LinearMenu m_assistTick;
	LinearMenu m_autoSync;
	LinearMenu m_fastSlow;
	LinearMenu m_noteSkin;
	LinearMenu m_movie;

	// BT-Dメニュー(ハイスピード)用
	ArrayWithLinearMenu<MusicGame::HispeedType> m_hispeedTypeMenu;
	LinearMenu m_hispeedValueMenu;

	void refreshHispeedValueMenuConstraints();

	// 現在メニューが表示されているか
	bool m_isVisible = false;

	// 現在押されているBTボタンを取得(単独押しの場合のみ)
	[[nodiscard]]
	Optional<Button> getCurrentSingleBTButton() const;

	// BT-Aメニューの表示テキスト生成
	[[nodiscard]]
	String generateBTAMenuText() const;

	// BT-Bメニューの表示テキスト生成
	[[nodiscard]]
	String generateBTBMenuText() const;

	// BT-Cメニューの表示テキスト生成
	[[nodiscard]]
	String generateBTCMenuText() const;

	// BT-Dメニューの表示テキスト生成
	[[nodiscard]]
	String generateBTDMenuText() const;

public:
	explicit BTOptionPanel(std::shared_ptr<noco::Canvas> canvas);

	// BTメニューの更新
	// ハイスコア再更新が必要な設定変更があった場合はtrueを返す
	bool update(double currentChartStdBPM);

	// メニューが表示されているかどうか
	[[nodiscard]]
	bool isVisible() const;

	// パネルを非表示にする
	void hide();

	// ConfigIniから設定値を読み込む
	void loadFromConfigIni();

	// ConfigIniに設定値を保存する
	void saveToConfigIni();
};
