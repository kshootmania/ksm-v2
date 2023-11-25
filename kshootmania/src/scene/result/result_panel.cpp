#include "result_panel.hpp"
#include "graphics/number_texture_font.hpp"

namespace
{
	constexpr Size kPanelSize = Size{ 556, 700 } * 2;

	constexpr Size kTopSize = Size{ 600, 50 } * 2;

	int32 GradeToTextureRow(Grade grade)
	{
		assert(grade != Grade::kNoGrade && "Unexpected grade in play result: Grade::kNoGrade");

		return static_cast<int32>(grade) - 1;
	}

	RenderTexture CreateRenderTexture(FilePathView chartFilePath, const kson::ChartData& chartData, const MusicGame::PlayResult& playResult)
	{
		using namespace MusicGame;

		const Texture panelTextureAsset = TextureAsset(ResultTexture::kPanel);
		const NumberTextureFont scoreFont(ResultTexture::kScoreNumberFont, { 64, 64 });
		const NumberTextureFont maxComboFont(ResultTexture::kMaxComboFont, { 136, 120 });
		const NumberTextureFont judgmentCountFont(ResultTexture::kJudgmentCountFont, { 136, 120 });
		const NumberTextureFont gaugePercentFont(ResultTexture::kPercentNumberTextureFont, { 136, 120 });

		const Size panelSize = panelTextureAsset.size();
		RenderTexture renderTexture(panelSize);
		Shader::Copy(panelTextureAsset, renderTexture);

		const ScopedRenderTarget2D renderTarget(renderTexture);
		const ScopedRenderStates2D renderState(SamplerState::ClampAniso);

		// 曲名/アーティスト名を表示
		const Font font = FontAsset(kFontAssetSystem);
		font(Unicode::FromUTF8(chartData.meta.title)).drawAt(28, 303, 26);
		font(Unicode::FromUTF8(chartData.meta.artist)).drawAt(20, 303, 61);

		// ジャケットを表示
		const FilePath parentPath = FileSystem::ParentPath(chartFilePath);
		const Texture jacketTexture(parentPath + Unicode::FromUTF8(chartData.meta.jacketFilename));
		if (!jacketTexture.isEmpty())
		{
			jacketTexture.resized(246, 246).draw(180, 85);
		}

		// 難易度を表示
		const TiledTexture difficultyTiledTexture(ResultTexture::kDifficulty, TiledTextureSizeInfo{
			.row = kNumDifficulties,
			.sourceSize = { 84, 24 },
		});
		difficultyTiledTexture(chartData.meta.difficulty.idx).scaled(0.8).draw(Point{ 94, 366 });

		// レベルを表示
		const TiledTexture levelTiledTexture(ResultTexture::kLevel, TiledTextureSizeInfo{
			.row = kLevelMax,
			.sourceSize = { 150, 120 },
		});
		levelTiledTexture(chartData.meta.level - 1).scaled(0.25).draw(Point{ 210, 361 });

		// スコアを表示
		const TextureFontTextLayout scoreTextLayout({ 64, 64 }, TextureFontTextLayout::Align::Left, 8, 50.0);
		scoreFont.draw(scoreTextLayout, { 100, 400 }, playResult.score, ZeroPaddingYN::Yes);

		// 最大コンボ数を表示
		const TextureFontTextLayout maxComboTextLayout({ 20, 20 }, TextureFontTextLayout::Align::Right, 4, 27.0);
		maxComboFont.draw(maxComboTextLayout, { 400, 472 }, playResult.maxCombo, ZeroPaddingYN::Yes);

		// 判定内訳の数字を表示
		const TextureFontTextLayout judgmentCountTextLayout({ 26, 24 }, TextureFontTextLayout::Align::Right, 4, 30.0);
		judgmentCountFont.draw(judgmentCountTextLayout, { 354, 532 + 54 * 0 }, playResult.comboStats.critical, ZeroPaddingYN::Yes);
		judgmentCountFont.draw(judgmentCountTextLayout, { 354, 532 + 54 * 1 }, playResult.comboStats.near(), ZeroPaddingYN::Yes);
		judgmentCountFont.draw(judgmentCountTextLayout, { 354, 532 + 54 * 2 }, playResult.comboStats.error, ZeroPaddingYN::Yes);

		// グレードを表示
		const TiledTexture gradeTiledTexture(ResultTexture::kGrade, TiledTextureSizeInfo{
			.row = 6,
			.sourceSize = { 100, 100 },
		});
		gradeTiledTexture(GradeToTextureRow(playResult.grade())).draw(Point{ 430, 286 });

		// ゲージのパーセンテージを表示
		const TextureFontTextLayout gaugePercentTextLayout({ 13, 15 }, TextureFontTextLayout::Align::Right, 3, 14.0);
		gaugePercentFont.draw(gaugePercentTextLayout, { 0, 445 }, static_cast<int32>(playResult.gaugePercentage)/*TODO: Hard gauge*/, ZeroPaddingYN::No);

		// ゲージのバーを表示
		constexpr Size kGaugeBarTextureSize{ 48, 434 };
		constexpr Size kGaugeBarSize{ 48, 435 }; // 描画先(result.png)のゲージ部分はミスって素材(er_g.gif)より1px大きくなっている
		const TiledTexture gaugeBarTiledTexture(ResultTexture::kGaugeBarTextureFilename, TiledTextureSizeInfo{
			.column = kNumGaugeTypes * 2,
			.sourceSize = kGaugeBarTextureSize,
		});
		constexpr Vec2 kGaugeBarPosition{ 4, 4 };
		const double percentThreshold = (playResult.gaugeType == GaugeType::kHardGauge) ? kGaugePercentageThresholdHardWarning : kGaugePercentageThreshold;
		const int32 gaugeBarColumn = static_cast<int32>(playResult.gaugeType) * 2 + ((playResult.gaugePercentage < percentThreshold) ? 0 : 1);
		const int32 gaugeBarHeight = static_cast<int32>(kGaugeBarSize.y * playResult.gaugePercentage / 100);
		gaugeBarTiledTexture(0, gaugeBarColumn).resized(kGaugeBarSize).drawClipped(kGaugeBarPosition, { kGaugeBarPosition.x, kGaugeBarPosition.y + kGaugeBarSize.y - gaugeBarHeight, kGaugeBarSize.x, gaugeBarHeight });

		return renderTexture;
	}

	enum TopTextureRow : int32
	{
		kTopTextureRowFailed = 0,
		kTopTextureRowCleared,
		kTopTextureRowFullCombo,
		kTopTextureRowPerfect,
		kTopTextureRowAssistFailed,
		kTopTextureRowAssistClear,
		kTopTextureRowAssistFullCombo,
		kTopTextureRowAssistPerfect,
		kTopTextureRowEasyFailed,
		kTopTextureRowEasyClear,
		kTopTextureRowEasyFullCombo,
		kTopTextureRowEasyPerfect,
		kTopTextureRowAssistEasyFailed,
		kTopTextureRowAssistEasyClear,
		kTopTextureRowAssistEasyFullCombo,
		kTopTextureRowAssistEasyPerfect,
		kTopTextureRowHardFailed,
		kTopTextureRowHardClear,
		kTopTextureRowAssistHardFailed,
		kTopTextureRowAssistHardClear,
	};

	int32 TopTextureRow(const MusicGame::PlayResult& playResult)
	{
		const Achievement achievement = playResult.achievement();
		const bool isEasyGauge = playResult.gaugeType == GaugeType::kEasyGauge;
		const bool isHardGauge = playResult.gaugeType == GaugeType::kHardGauge;
		const bool isAssist = false; // TODO: アシストかどうかを代入
		switch (achievement)
		{
		case Achievement::kNone:
			if (isAssist)
			{
				switch (playResult.gaugeType)
				{
				case GaugeType::kNormalGauge:
					return kTopTextureRowAssistFailed;

				case GaugeType::kEasyGauge:
					return kTopTextureRowAssistEasyFailed;

				case GaugeType::kHardGauge:
					return kTopTextureRowAssistHardFailed;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			else
			{
				switch (playResult.gaugeType)
				{
				case GaugeType::kNormalGauge:
					return kTopTextureRowFailed;

				case GaugeType::kEasyGauge:
					return kTopTextureRowEasyFailed;

				case GaugeType::kHardGauge:
					return kTopTextureRowHardFailed;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			break;

		case Achievement::kCleared:
			if (isAssist)
			{
				switch (playResult.gaugeType)
				{
				case GaugeType::kNormalGauge:
					return kTopTextureRowAssistClear;

				case GaugeType::kEasyGauge:
					return kTopTextureRowAssistEasyClear;

				case GaugeType::kHardGauge:
					return kTopTextureRowAssistHardClear;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			else
			{
				switch (playResult.gaugeType)
				{
				case GaugeType::kNormalGauge:
					return kTopTextureRowCleared;

				case GaugeType::kEasyGauge:
					return kTopTextureRowEasyClear;

				case GaugeType::kHardGauge:
					return kTopTextureRowHardClear;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			break;

		case Achievement::kFullCombo:
			if (isAssist)
			{
				switch (playResult.gaugeType)
				{
				case GaugeType::kNormalGauge:
				case GaugeType::kHardGauge:
					return kTopTextureRowAssistFullCombo;

				case GaugeType::kEasyGauge:
					return kTopTextureRowAssistEasyFullCombo;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			else
			{
				switch (playResult.gaugeType)
				{
				case GaugeType::kNormalGauge:
				case GaugeType::kHardGauge:
					return kTopTextureRowFullCombo;

				case GaugeType::kEasyGauge:
					return kTopTextureRowEasyFullCombo;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			break;

		case Achievement::kPerfect:
			if (isAssist)
			{
				switch (playResult.gaugeType)
				{
				case GaugeType::kNormalGauge:
				case GaugeType::kHardGauge:
					return kTopTextureRowAssistPerfect;

				case GaugeType::kEasyGauge:
					return kTopTextureRowAssistEasyPerfect;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			else
			{
				switch (playResult.gaugeType)
				{
				case GaugeType::kNormalGauge:
				case GaugeType::kHardGauge:
					return kTopTextureRowPerfect;

				case GaugeType::kEasyGauge:
					return kTopTextureRowEasyPerfect;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			break;

		default:
			assert(false && "Unexpected achievement in play result");
		}

		assert(false && "This line should never be reached");
		return kTopTextureRowFailed;
	}
}

ResultPanel::ResultPanel(FilePathView chartFilePath, const kson::ChartData& chartData, const MusicGame::PlayResult& playResult)
	: m_renderTexture(CreateRenderTexture(chartFilePath, chartData, playResult))
	, m_topTextureRow(TopTextureRow(playResult))
{
}

void ResultPanel::draw() const
{
	using namespace ScreenUtils;

	m_renderTexture.resized(ScaledL(kPanelSize)).draw(Scaled(140) + LeftMargin(), Scaled(60));
	m_topTexture(m_topTextureRow).resized(ScaledL(kTopSize)).draw(Scaled(138) + LeftMargin(), Scaled(20));
}
