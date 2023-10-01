#pragma once
#include "result_assets.hpp"
#include "music_game/play_result.hpp"

class ResultPanel
{
private:
	const RenderTexture m_renderTexture;

	const TiledTexture m_topTexture
	{
		U"result_top.png",
		TiledTextureSizeInfo
		{
			.row = 20,
			.sourceSize = { 600, 50 },
		}
	};

	const int32 m_topTextureRow;

public:
	ResultPanel(FilePathView chartFilePath, const kson::ChartData& chartData, const MusicGame::PlayResult& playResult);

	void draw() const;
};
